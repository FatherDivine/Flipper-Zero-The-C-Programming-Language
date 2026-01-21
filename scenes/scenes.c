#include "scenes.h"
#include "../callbacks/callbacks.h"
#include "../resource/resource.h"
#include "../constants/constants.h"
#include "../buffer/dynamic_buffer.h"
#include <string.h>

/* Display parameters for Flipper Zero screen (128x64 pixels)
 * 
 * The screen is divided into content area and footer area:
 * - Content area: Lines 1-5 (main text from the book)
 * - Footer area: Line 6 (navigation hints and page numbers)
 *   Note: The footer is sometimes called the "lower third" as it occupies
 *   the bottom portion of the screen with navigation/status information
 * 
 * The footer contains:
 * - Left arrow '<' indicating previous page navigation
 * - Right arrow '>' indicating next page navigation  
 * - Page counter [current/total] in the center
 * - Optional bookmark indicator '*' when page is bookmarked
 */
#define CHARS_PER_LINE 25   // Character width: ~128px / 5px per char = 25 chars
#define LINES_PER_PAGE 5    // Content lines: Increased to 5 for more text per page

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

/* Calculate page layout with proper line wrapping
 * 
 * This function determines how many bytes of text fit on a single page
 * while respecting word boundaries and line length constraints.
 * 
 * Parameters:
 *   buffer - Text buffer to paginate
 *   buffer_len - Length of the buffer in bytes
 *   chars_per_line - Maximum characters per line (25 for Flipper Zero)
 *   max_lines - Maximum lines per page (5 for content area)
 * 
 * Returns:
 *   Number of bytes that fit on this page, preferring to break at word boundaries
 * 
 * Algorithm:
 *   - Tracks current column and line position
 *   - Respects explicit newlines in the text
 *   - Wraps long lines at word boundaries (spaces) when possible
 *   - Returns early if max_lines is reached
 */
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

/* Initialize file and calculate total pages
 * 
 * This function pre-scans the entire file to:
 * 1. Determine the total number of pages
 * 2. Store the byte offset where each page starts (in page_offsets array)
 * 
 * This allows for accurate page numbering and direct page navigation.
 * The scan respects the same line wrapping rules as display, ensuring
 * consistent pagination throughout the reading experience.
 * 
 * Returns true on success, false on file access error
 */
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

/* Load current page content into display buffer
 * 
 * This function:
 * 1. Reads content from file at the current offset
 * 2. Calculates how much content fits on this page
 * 3. Builds the display buffer with content and footer
 * 
 * The display buffer structure:
 * - Lines 1-5: Book content (paginated text)
 * - Line 6: Footer/status line (navigation and page numbers)
 * 
 * Footer format:
 * - Without bookmark: "<   [page/total]   >"
 * - With bookmark:    "< * [page/total] >"
 * 
 * Returns true on success, false on file access error
 */
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
    
    // Copy page content (lines 1-5)
    size_t display_pos = 0;
    for(size_t i = 0; i < page_content_len && display_pos < DISPLAY_BUFFER_SIZE - 64; i++) {
        app->display_buffer[display_pos++] = app->page_buffer[i];
    }
    
    // Count how many lines the content actually used
    // This determines how many padding newlines we need before the footer
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
    
    // Add padding newlines to push footer to line 6
    // We want the footer on line 6, so pad to fill lines up to line 5
    while(content_lines < LINES_PER_PAGE && display_pos < DISPLAY_BUFFER_SIZE - 32) {
        app->display_buffer[display_pos++] = '\n';
        content_lines++;
    }
    
    // Ensure there's a newline before the footer (separates content from footer)
    if(display_pos > 0 && app->display_buffer[display_pos - 1] != '\n') {
        app->display_buffer[display_pos++] = '\n';
    }
    
    // Check if current page is bookmarked (for footer display)
    app->current_page_bookmarked = app_is_page_bookmarked(app);
    
    /* Build the footer (status line) on line 6
     * Footer shows: navigation arrows + page numbers + bookmark indicator
     * Format examples:
     *   "<   [1/42]   >"  - Normal page
     *   "< * [5/42] >"    - Bookmarked page (asterisk indicates bookmark)
     */
    char footer[64];
    if(app->current_page_bookmarked) {
        snprintf(footer, sizeof(footer), "< * [%zu/%zu] >", 
                app->current_page + 1, app->total_pages);
    } else {
        snprintf(footer, sizeof(footer), "<   [%zu/%zu]   >", 
                app->current_page + 1, app->total_pages);
    }
    
    // Center the footer horizontally on the line
    size_t footer_len = strlen(footer);
    size_t padding = (CHARS_PER_LINE - footer_len) / 2;
    for(size_t i = 0; i < padding && display_pos < DISPLAY_BUFFER_SIZE - footer_len - 1; i++) {
        app->display_buffer[display_pos++] = ' ';
    }
    
    // Append the centered footer text
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

/* Helper function to navigate to a specific topic
 * Resets pagination state and refreshes the scene
 */
static void navigate_to_topic(App* app, size_t chapter_index, size_t topic_index) {
    app->current_chapter_index = chapter_index;
    app->chapter_selected_index = topic_index;
    app->current_topic = chapters[chapter_index].content[topic_index].file_path;
    app->file_offset = 0;
    app->total_pages = 0;
    app->current_page = 0;
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
            // Check if we're at the last page of this topic
            if(app->current_page < app->total_pages - 1) {
                // Not at end - advance to next page within this topic
                app->current_page++;
                app->file_offset = app->page_offsets[app->current_page];
                // Refresh the current scene
                topic_scene_on_exit(app);
                topic_scene_on_enter(app);
            } else {
                // At last page of this topic - try to roll over to next topic/chapter
                // This implements "like a real book" behavior
                
                size_t current_chapter = app->current_chapter_index;
                size_t current_topic = app->chapter_selected_index;
                Chapter curr_chapter = chapters[current_chapter];
                
                // Check if there's a next topic in the current chapter
                if(current_topic + 1 < curr_chapter.number_of_topics) {
                    // Move to next topic in same chapter
                    navigate_to_topic(app, current_chapter, current_topic + 1);
                    topic_scene_on_exit(app);
                    topic_scene_on_enter(app);
                } else if(current_chapter + 1 < number_of_chapters) {
                    // Move to first topic of next chapter
                    Chapter next_chapter = chapters[current_chapter + 1];
                    if(next_chapter.number_of_topics > 0) {
                        navigate_to_topic(app, current_chapter + 1, 0);
                        topic_scene_on_exit(app);
                        topic_scene_on_enter(app);
                    }
                }
                // If we're at the very last page of the last chapter, do nothing
                // (already at the end of the book)
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