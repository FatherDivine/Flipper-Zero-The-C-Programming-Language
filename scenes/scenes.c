#include "scenes.h"
#include "../callbacks/callbacks.h"
#include "../resource/resource.h"
#include "../constants/constants.h"
#include "../buffer/dynamic_buffer.h"
#include <string.h>

// Display parameters for Flipper Zero screen
// Using 4 lines for content + 1 line for footer (page numbers, navigation)
#define CHARS_PER_LINE 25   // ~128px / 5px per char
#define LINES_PER_PAGE 4    // 4 lines for content, leaving room for footer

// Start Menu Scene - new main entry point
void start_menu_scene_on_enter(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "C Programming Book");

    uint32_t idx = 0;
    
    // Show "Continue Reading" only if there's a saved position
    if(app->has_reading_position && strlen(app->last_topic_path) > 0) {
        submenu_add_item(app->submenu, "Continue Reading", StartMenuContinue, start_menu_callback, app);
        idx++;
    }
    
    submenu_add_item(app->submenu, "Read from Beginning", StartMenuRead, start_menu_callback, app);
    submenu_add_item(app->submenu, "Table of Contents", StartMenuTOC, start_menu_callback, app);
    submenu_add_item(app->submenu, "Bookmarks", StartMenuBookmarks, start_menu_callback, app);
    submenu_add_item(app->submenu, "Options", StartMenuOptions, start_menu_callback, app);
    submenu_add_item(app->submenu, "About / Credits", StartMenuCredits, start_menu_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, SubmenuView);
}

bool start_menu_scene_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void start_menu_scene_on_exit(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
}

// Table of Contents (formerly main menu)
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

// Options Scene
void options_scene_on_enter(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "Options");

    // Backlight option
    if(app->backlight_on) {
        submenu_add_item(app->submenu, "Backlight: ON", OptionsBacklight, options_callback, app);
    } else {
        submenu_add_item(app->submenu, "Backlight: Auto", OptionsBacklight, options_callback, app);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, SubmenuView);
}

bool options_scene_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;
    
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == OptionsBacklight) {
            // Toggle backlight
            app->backlight_on = !app->backlight_on;
            app_set_backlight(app, app->backlight_on);
            app_save_settings(app);
            // Refresh the scene to show updated state
            options_scene_on_exit(app);
            options_scene_on_enter(app);
            consumed = true;
        }
    }
    
    return consumed;
}

void options_scene_on_exit(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
}

// Credits Scene
void credits_scene_on_enter(void* context) {
    App* app = context;
    widget_reset(app->widget);

    const char* credits_text = 
        "The C Programming Language\n"
        "Flipper Zero Edition\n"
        "\n"
        "Original Book by:\n"
        "Brian W. Kernighan\n"
        "Dennis M. Ritchie\n"
        "\n"
        "App Development:\n"
        "@armixz - Original idea,\n"
        "preface & Ch1 content\n"
        "\n"
        "@FatherDivine -\n"
        "Completion, polish,\n"
        "features & chapters\n"
        "\n"
        "Version 0.3";

    widget_add_text_scroll_element(
        app->widget, 0, 0, WIDGET_WIDTH, WIDGET_HEIGHT, credits_text);

    view_dispatcher_switch_to_view(app->view_dispatcher, WidgetView);
}

bool credits_scene_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void credits_scene_on_exit(void* context) {
    App* app = context;
    widget_reset(app->widget);
}

// Bookmarks Scene
void bookmarks_scene_on_enter(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "Bookmarks");

    if(app->bookmark_count == 0) {
        submenu_add_item(app->submenu, "(No bookmarks)", 0, NULL, app);
    } else {
        for(size_t i = 0; i < app->bookmark_count; i++) {
            // Extract filename from path for display
            const char* path = app->bookmark_topics[i];
            const char* filename = strrchr(path, '/');
            if(filename) {
                filename++; // Skip the '/'
            } else {
                filename = path;
            }
            submenu_add_item(app->submenu, filename, i, bookmarks_callback, app);
        }
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, SubmenuView);
}

bool bookmarks_scene_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void bookmarks_scene_on_exit(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
}

// Calculate page layout with proper line wrapping
// Returns the number of bytes that fit on this page
static size_t calculate_page_content(const char* buffer, size_t buffer_len, size_t chars_per_line, size_t max_lines) {
    if(buffer_len == 0) return 0;
    
    size_t lines_used = 0;
    size_t col = 0;
    size_t i = 0;
    size_t last_space_pos = 0;
    size_t last_line_start = 0;
    
    while(i < buffer_len && lines_used < max_lines) {
        char c = buffer[i];
        
        if(c == '\n') {
            // Newline - move to next line
            lines_used++;
            col = 0;
            last_line_start = i + 1;
            i++;
            continue;
        }
        
        if(c == ' ') {
            last_space_pos = i;
        }
        
        col++;
        
        if(col > chars_per_line) {
            // Line too long, need to wrap
            lines_used++;
            
            if(lines_used >= max_lines) {
                // We've hit the limit, return up to last good break point
                if(last_space_pos > last_line_start) {
                    return last_space_pos + 1; // Include the space
                }
                return i;
            }
            
            // Try to wrap at word boundary
            if(last_space_pos > last_line_start && c != ' ') {
                // Go back to the space and wrap there
                i = last_space_pos + 1;
                col = 0;
                last_line_start = i;
                continue;
            }
            
            col = 1;
            last_line_start = i;
        }
        
        i++;
    }
    
    // Check if we've used all lines
    if(lines_used >= max_lines) {
        // Return to last word boundary if possible
        if(last_space_pos > last_line_start) {
            return last_space_pos + 1;
        }
    }
    
    return i;
}

// Initialize file and calculate total pages
static bool init_file_pages(App* app) {
    const char* file_path = app->current_topic;
    
    // Check SD card first
    if(!app_check_sd_card(app)) {
        return false;
    }
    
    if(!file_stream_open(app->file_stream, file_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        app->file_size = 0;
        app->total_pages = 1;
        return false;
    }
    
    // Get file size
    stream_seek(app->file_stream, 0, StreamOffsetFromEnd);
    app->file_size = stream_tell(app->file_stream);
    stream_seek(app->file_stream, 0, StreamOffsetFromStart);
    
    // Calculate pages by scanning through file
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
        size_t page_len = calculate_page_content(app->page_buffer, bytes_read, CHARS_PER_LINE, LINES_PER_PAGE);
        if(page_len == 0) page_len = bytes_read; // Safety: advance at least some
        
        offset += page_len;
        
        // Skip leading whitespace at page boundary
        while(offset < app->file_size) {
            stream_seek(app->file_stream, offset, StreamOffsetFromStart);
            char c;
            if(stream_read(app->file_stream, (uint8_t*)&c, 1) == 0) break;
            if(c == ' ') {
                offset++;
            } else {
                break;
            }
        }
    }
    
    if(app->total_pages == 0) app->total_pages = 1;
    
    file_stream_close(app->file_stream);
    return true;
}

// Load current page content into display buffer
static bool load_current_page(App* app) {
    const char* file_path = app->current_topic;
    
    // Check SD card first
    if(!app_check_sd_card(app)) {
        return false;
    }
    
    if(!file_stream_open(app->file_stream, file_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        return false;
    }
    
    // Seek to current page offset
    stream_seek(app->file_stream, app->file_offset, StreamOffsetFromStart);
    
    // Read content into buffer
    size_t bytes_read = stream_read(app->file_stream, (uint8_t*)app->page_buffer, PAGE_BUFFER_SIZE - 1);
    file_stream_close(app->file_stream);
    
    if(bytes_read == 0) {
        return false;
    }
    
    app->page_buffer[bytes_read] = '\0';
    
    // Calculate how much content fits on this page
    size_t page_content_len = calculate_page_content(app->page_buffer, bytes_read, CHARS_PER_LINE, LINES_PER_PAGE);
    app->current_page_size = page_content_len;
    
    // Build display buffer with content and footer
    memset(app->display_buffer, 0, DISPLAY_BUFFER_SIZE);
    
    // Copy page content
    size_t display_pos = 0;
    for(size_t i = 0; i < page_content_len && display_pos < DISPLAY_BUFFER_SIZE - 64; i++) {
        app->display_buffer[display_pos++] = app->page_buffer[i];
    }
    
    // Add newlines to ensure footer is at bottom
    // Count existing newlines in content
    size_t content_lines = 1;
    size_t col_count = 0;
    for(size_t i = 0; i < page_content_len; i++) {
        if(app->page_buffer[i] == '\n') {
            content_lines++;
            col_count = 0;
        } else {
            col_count++;
            if(col_count >= CHARS_PER_LINE) {
                content_lines++;
                col_count = 0;
            }
        }
    }
    
    // Add padding newlines if needed (we want footer on line 5)
    while(content_lines < LINES_PER_PAGE && display_pos < DISPLAY_BUFFER_SIZE - 32) {
        app->display_buffer[display_pos++] = '\n';
        content_lines++;
    }
    
    // Ensure there's a newline before footer
    if(display_pos > 0 && app->display_buffer[display_pos - 1] != '\n') {
        app->display_buffer[display_pos++] = '\n';
    }
    
    // Check if page is bookmarked
    app->current_page_bookmarked = app_is_page_bookmarked(app);
    
    // Add footer: navigation hints and page number
    // Format: < [page/total] > or *< [page/total] > if bookmarked
    char footer[64];
    if(app->current_page_bookmarked) {
        snprintf(footer, sizeof(footer), "< * [%zu/%zu] >", 
                app->current_page + 1, app->total_pages);
    } else {
        snprintf(footer, sizeof(footer), "<   [%zu/%zu]   >", 
                app->current_page + 1, app->total_pages);
    }
    
    // Center the footer
    size_t footer_len = strlen(footer);
    size_t padding = (CHARS_PER_LINE - footer_len) / 2;
    for(size_t i = 0; i < padding && display_pos < DISPLAY_BUFFER_SIZE - footer_len - 1; i++) {
        app->display_buffer[display_pos++] = ' ';
    }
    
    // Append footer
    for(size_t i = 0; i < footer_len && display_pos < DISPLAY_BUFFER_SIZE - 1; i++) {
        app->display_buffer[display_pos++] = footer[i];
    }
    
    app->display_buffer[display_pos] = '\0';
    
    return true;
}

// Find page index from offset
static size_t find_page_from_offset(App* app, size_t offset) {
    for(size_t i = 0; i < app->total_pages; i++) {
        if(app->page_offsets[i] == offset) {
            return i;
        }
        if(i + 1 < app->total_pages && app->page_offsets[i + 1] > offset) {
            return i;
        }
    }
    return app->total_pages > 0 ? app->total_pages - 1 : 0;
}

// Paged topic viewer: reads one page from file_offset into page_buffer
void topic_scene_on_enter(void* context) {
    furi_assert(context);
    App* app = (App*)context;
    widget_reset(app->widget);

    // Check SD card availability
    if(!app_check_sd_card(app)) {
        widget_add_text_scroll_element(
            app->widget, 0, 0, WIDGET_WIDTH, WIDGET_HEIGHT, 
            "SD Card Error!\n\nSD card not found or\nremoved.\n\nPlease insert SD card\nand try again.");
        view_dispatcher_switch_to_view(app->view_dispatcher, WidgetView);
        return;
    }

    // If this is first entry, initialize pages
    if(app->total_pages == 0) {
        if(!init_file_pages(app)) {
            widget_add_text_scroll_element(
                app->widget, 0, 0, WIDGET_WIDTH, WIDGET_HEIGHT, 
                "Failed to open file.\n\nFile may be missing\nor corrupted.");
            view_dispatcher_switch_to_view(app->view_dispatcher, WidgetView);
            return;
        }
        app->current_page = find_page_from_offset(app, app->file_offset);
    }

    // Enable backlight if setting is on
    if(app->backlight_on) {
        app_set_backlight(app, true);
    }

    // Load current page
    if(!load_current_page(app)) {
        widget_add_text_scroll_element(
            app->widget, 0, 0, WIDGET_WIDTH, WIDGET_HEIGHT, 
            "Failed to load page.\n\nSD card may have been\nremoved.");
        view_dispatcher_switch_to_view(app->view_dispatcher, WidgetView);
        return;
    }

    // Save reading position
    app_save_reading_position(app);

    widget_add_text_scroll_element(
        app->widget, 0, 0, WIDGET_WIDTH, WIDGET_HEIGHT, app->display_buffer);

    view_dispatcher_switch_to_view(app->view_dispatcher, WidgetView);
}

bool topic_scene_on_event(void* context, SceneManagerEvent event) {
    App* app = context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NextPageEvent) {
            // Go to next page if not at end
            if(app->current_page < app->total_pages - 1) {
                app->current_page++;
                app->file_offset = app->page_offsets[app->current_page];
                // Refresh the current scene
                topic_scene_on_exit(app);
                topic_scene_on_enter(app);
            }
            return true;
        }
        if(event.event == PrevPageEvent) {
            // Go to previous page if not at beginning
            if(app->current_page > 0) {
                app->current_page--;
                app->file_offset = app->page_offsets[app->current_page];
                // Refresh the current scene
                topic_scene_on_exit(app);
                topic_scene_on_enter(app);
            }
            return true;
        }
        if(event.event == BookmarkEvent) {
            // Toggle bookmark
            if(app->current_page_bookmarked) {
                app_remove_bookmark(app);
            } else {
                app_add_bookmark(app);
            }
            // Refresh to show bookmark status
            topic_scene_on_exit(app);
            topic_scene_on_enter(app);
            return true;
        }
    }

    return false;
}

void topic_scene_on_exit(void* context) {
    furi_assert(context);
    App* app = (App*)context;
    widget_reset(app->widget);
    
    // Save position when leaving
    app_save_reading_position(app);
}