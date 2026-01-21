#include "scenes.h"
#include "../callbacks/callbacks.h"
#include "../resource/resource.h"
#include "../constants/constants.h"
#include "../buffer/dynamic_buffer.h"

// Display parameters for Flipper Zero screen
#define CHARS_PER_LINE 25   // ~128px / 5px per char
#define LINES_PER_PAGE 7    // ~64px / 9px per line (with some margin for scroll bar)

void main_menu_scene_on_enter(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "Table of Contents");

    for(size_t i = 0; i < number_of_chapters; i++) {
        submenu_add_item(app->submenu, chapters[i].name, i, menu_callback, app);
    }

    // Restore last selected chapter
    submenu_set_selected_item(app->submenu, app->main_menu_selected_index);

    view_dispatcher_switch_to_view(app->view_dispatcher, SubmenuView);
}

bool main_menu_scene_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;
    switch(event.type) {
    case SceneManagerEventTypeCustom:
        switch(event.event) {
        case MainMenuEvent:
            scene_manager_next_scene(app->scene_manager, ChapterScene);
            consumed = true;
            break;
        }
        break;
    default:
        break;
    }
    return consumed;
}

void main_menu_scene_on_exit(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
}

void chapter_scene_on_enter(void* context) {
    App* app = (App*)context;
    size_t chapterIndex = app->current_chapter_index;
    Chapter currentChapter = chapters[chapterIndex];

    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, currentChapter.name);

    for(size_t i = 0; i < currentChapter.number_of_topics; i++) {
        const char* label = currentChapter.content[i].name;
        submenu_add_item(app->submenu, label, i, chapter_callback, app);
    }

    // Restore last selected topic within this chapter
    submenu_set_selected_item(app->submenu, app->chapter_selected_index);

    view_dispatcher_switch_to_view(app->view_dispatcher, SubmenuView);
}

bool chapter_scene_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void chapter_scene_on_exit(void* context) {
    UNUSED(context);
}

// Find the byte offset where we should end this page (end at word boundary)
// Returns number of bytes that fit in the page
static size_t find_page_end(const char* buffer, size_t buffer_len, size_t chars_per_line, size_t max_lines) {
    if(buffer_len == 0) return 0;
    
    size_t lines = 1;
    size_t col = 0;
    size_t last_space = 0;
    size_t last_line_start = 0;
    
    for(size_t i = 0; i < buffer_len; i++) {
        char c = buffer[i];
        
        if(c == '\n') {
            lines++;
            col = 0;
            last_line_start = i + 1;
            if(lines > max_lines) {
                // Return up to this newline
                return i;
            }
        } else if(c == ' ') {
            last_space = i;
            col++;
            if(col >= chars_per_line) {
                lines++;
                col = 0;
                last_line_start = i + 1;
                if(lines > max_lines) {
                    return i;
                }
            }
        } else {
            col++;
            if(col >= chars_per_line) {
                lines++;
                col = 0;
                last_line_start = i + 1;
                if(lines > max_lines) {
                    // Try to break at last space if available and on same wrapped block
                    if(last_space > 0 && last_space > last_line_start - chars_per_line) {
                        return last_space;
                    }
                    return i;
                }
            }
        }
    }
    
    // All text fits
    return buffer_len;
}

// Initialize file and calculate total pages
static void init_file_pages(App* app) {
    const char* file_path = app->current_topic;
    
    if(!file_stream_open(app->file_stream, file_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        app->file_size = 0;
        app->total_pages = 1;
        return;
    }
    
    // Get file size
    stream_seek(app->file_stream, 0, StreamOffsetFromEnd);
    app->file_size = stream_tell(app->file_stream);
    stream_seek(app->file_stream, 0, StreamOffsetFromStart);
    
    // Calculate pages by scanning through file
    // Reuse app->page_buffer to avoid large stack allocation
    app->total_pages = 0;
    size_t offset = 0;
    
    while(offset < app->file_size && app->total_pages < MAX_PAGE_HISTORY) {
        // Store this page's start offset
        app->page_offsets[app->total_pages] = offset;
        app->total_pages++;
        
        // Read a chunk into app->page_buffer
        stream_seek(app->file_stream, offset, StreamOffsetFromStart);
        size_t bytes_read = stream_read(app->file_stream, (uint8_t*)app->page_buffer, PAGE_BUFFER_SIZE - 1);
        if(bytes_read == 0) break;
        app->page_buffer[bytes_read] = '\0';
        
        // Find where this page ends
        size_t page_len = find_page_end(app->page_buffer, bytes_read, CHARS_PER_LINE, LINES_PER_PAGE);
        if(page_len == 0) page_len = bytes_read; // Safety: advance at least some
        
        offset += page_len;
        
        // Skip whitespace at page boundary to avoid leading spaces on next page
        while(offset < app->file_size) {
            stream_seek(app->file_stream, offset, StreamOffsetFromStart);
            char c;
            if(stream_read(app->file_stream, (uint8_t*)&c, 1) == 0) break;
            if(c == ' ' || c == '\n') {
                offset++;
            } else {
                break;
            }
        }
    }
    
    if(app->total_pages == 0) app->total_pages = 1;
    
    file_stream_close(app->file_stream);
}

// Load current page into buffer
static bool load_current_page(App* app) {
    const char* file_path = app->current_topic;
    
    if(!file_stream_open(app->file_stream, file_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        return false;
    }
    
    // Get the offset for current page
    size_t offset = 0;
    if(app->current_page < app->total_pages) {
        offset = app->page_offsets[app->current_page];
    }
    app->file_offset = offset;
    
    // Read the page content
    stream_seek(app->file_stream, offset, StreamOffsetFromStart);
    size_t bytes_read = stream_read(app->file_stream, (uint8_t*)app->page_buffer, PAGE_BUFFER_SIZE - 1);
    app->page_buffer[bytes_read] = '\0';
    
    // Trim to actual page length
    if(bytes_read > 0) {
        size_t page_len = find_page_end(app->page_buffer, bytes_read, CHARS_PER_LINE, LINES_PER_PAGE);
        if(page_len > 0 && page_len < bytes_read) {
            app->page_buffer[page_len] = '\0';
        }
    }
    
    file_stream_close(app->file_stream);
    return true;
}

// Build scroll indicator text showing position in file
static void build_page_indicator(App* app, char* indicator, size_t indicator_size) {
    // Create a simple page indicator like "Page 1/5"
    snprintf(indicator, indicator_size, "\n[%zu/%zu]", 
             app->current_page + 1, app->total_pages);
}

// Paged topic viewer with proper page tracking
void topic_scene_on_enter(void* context) {
    furi_assert(context);
    App* app = (App*)context;
    widget_reset(app->widget);

    // If this is first entry (file_offset == 0 and current_page == 0), initialize pages
    if(app->file_offset == 0 && app->current_page == 0 && app->total_pages == 0) {
        init_file_pages(app);
    }

    // Load current page
    if(!load_current_page(app)) {
        widget_add_text_scroll_element(
            app->widget, 0, 0, WIDGET_WIDTH, WIDGET_HEIGHT, "Failed to open file.");
        view_dispatcher_switch_to_view(app->view_dispatcher, WidgetView);
        return;
    }

    // Build display text with page indicator
    char indicator[32];
    build_page_indicator(app, indicator, sizeof(indicator));
    
    snprintf(app->display_buffer, DISPLAY_BUFFER_SIZE, "%s%s", app->page_buffer, indicator);

    widget_add_text_scroll_element(
        app->widget, 0, 0, WIDGET_WIDTH, WIDGET_HEIGHT, app->display_buffer);

    view_dispatcher_switch_to_view(app->view_dispatcher, WidgetView);
}

bool topic_scene_on_event(void* context, SceneManagerEvent event) {
    App* app = context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NextPageEvent) {
            // Go to next page if not at end
            if(app->current_page + 1 < app->total_pages) {
                app->current_page++;
                topic_scene_on_exit(app);
                topic_scene_on_enter(app);
            }
            return true;
        }
        if(event.event == PrevPageEvent) {
            // Go to previous page if not at start
            if(app->current_page > 0) {
                app->current_page--;
                topic_scene_on_exit(app);
                topic_scene_on_enter(app);
            }
            return true;
        }
    }

    return false;
}

void topic_scene_on_exit(void* context) {
    furi_assert(context);
    App* app = (App*)context;
    widget_reset(app->widget);
}