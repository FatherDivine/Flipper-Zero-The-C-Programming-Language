#include "scenes.h"
#include "../callbacks/callbacks.h"
#include "../resource/resource.h"
#include "../constants/constants.h"
#include "../buffer/dynamic_buffer.h"

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

char* wrap_text(const char* text, size_t max_line_width) {
    size_t len = strlen(text);
    size_t allocated_size = len / 2 + 1;
    char* wrapped = malloc(allocated_size);
    if(!wrapped) return NULL;

    size_t cur_line_len = 0;
    size_t wrapped_index = 0;
    size_t word_len = 0;

    for(size_t i = 0; i < len; ++i) {
        word_len++;
        if(text[i] == '\n') {
            for(size_t j = i - word_len + 1; j <= i; ++j) {
                if(wrapped_index >= allocated_size - 1) {
                    allocated_size *= 2;
                    char* new_wrapped = realloc(wrapped, allocated_size);
                    if(!new_wrapped) {
                        free(wrapped);
                        return NULL;
                    }
                    wrapped = new_wrapped;
                }
                wrapped[wrapped_index++] = text[j];
            }
            cur_line_len = 0;
            word_len = 0;
            continue;
        }

        if(text[i] == ' ' || i == len - 1) {
            if(word_len >= max_line_width) {
                if(cur_line_len > 0) {
                    wrapped[wrapped_index++] = '\n';
                    cur_line_len = 0;
                }
                for(size_t j = i - word_len + 1; j <= i; ++j) {
                    if(wrapped_index >= allocated_size - 1) {
                        allocated_size *= 2;
                        char* new_wrapped = realloc(wrapped, allocated_size);
                        if(!new_wrapped) {
                            free(wrapped);
                            return NULL;
                        }
                        wrapped = new_wrapped;
                    }
                    wrapped[wrapped_index++] = text[j];
                    if(++cur_line_len >= max_line_width && text[j] != '\n') {
                        wrapped[wrapped_index++] = '\n';
                        cur_line_len = 0;
                    }
                }
            } else if(cur_line_len + word_len > max_line_width) {
                if(cur_line_len > 0) {
                    wrapped[wrapped_index++] = '\n';
                    cur_line_len = 0;
                }
            }
            for(size_t j = i - word_len + 1; j <= i; ++j) {
                if(wrapped_index >= allocated_size - 1) {
                    allocated_size *= 2;
                    char* new_wrapped = realloc(wrapped, allocated_size);
                    if(!new_wrapped) {
                        free(wrapped);
                        return NULL;
                    }
                    wrapped = new_wrapped;
                }
                wrapped[wrapped_index++] = text[j];
            }
            cur_line_len += word_len;
            word_len = 0;
        }
    }

    wrapped[wrapped_index] = '\0';
    return wrapped;
}

// Find a good break point in the buffer, preferring paragraph/sentence/word boundaries
// Returns the position where the page should end (relative to buffer start)
static size_t find_page_break(const char* buffer, size_t bytes_read) {
    if(bytes_read == 0) return 0;
    
    // Target is roughly 80% of buffer to leave room for next page
    size_t target = (bytes_read * 4) / 5;
    if(target < 100) target = bytes_read; // If small, just use all of it
    
    // Search backwards from target for a good break point
    size_t best_break = target;
    
    // Look for paragraph break (double newline) - highest priority
    for(size_t i = target; i > target / 2 && i < bytes_read - 1; i--) {
        if(buffer[i] == '\n' && buffer[i + 1] == '\n') {
            return i + 2; // Include both newlines
        }
    }
    
    // Look for sentence end (period/question/exclamation followed by space/newline)
    for(size_t i = target; i > target / 2 && i < bytes_read - 1; i--) {
        if((buffer[i] == '.' || buffer[i] == '?' || buffer[i] == '!') &&
           (buffer[i + 1] == ' ' || buffer[i + 1] == '\n')) {
            return i + 2; // Include punctuation and space
        }
    }
    
    // Look for newline (end of line)
    for(size_t i = target; i > target / 2; i--) {
        if(buffer[i] == '\n') {
            return i + 1; // Include the newline
        }
    }
    
    // Look for word boundary (space)
    for(size_t i = target; i > target / 2; i--) {
        if(buffer[i] == ' ') {
            return i + 1; // Include the space
        }
    }
    
    // Fallback: use target position if no good break found
    return best_break;
}

// Paged topic viewer: reads one page from file_offset into page_buffer
void topic_scene_on_enter(void* context) {
    furi_assert(context);
    App* app = (App*)context;
    widget_reset(app->widget);

    const char* file_path = app->current_topic;

    if(!file_stream_open(app->file_stream, file_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        widget_add_text_scroll_element(
            app->widget, 0, 0, WIDGET_WIDTH, WIDGET_HEIGHT, "Failed to open asset file.");
        view_dispatcher_switch_to_view(app->view_dispatcher, WidgetView);
        return;
    }

    // Get file size to check if we're at the end
    size_t file_size = stream_size(app->file_stream);
    
    // If offset is beyond file size, reset to beginning
    if(app->file_offset >= file_size) {
        app->file_offset = 0;
    }

    // Seek to current page offset
    stream_seek(app->file_stream, app->file_offset, StreamOffsetFromStart);

    // Read one page into fixed buffer
    size_t bytes_read =
        stream_read(app->file_stream, (uint8_t*)app->page_buffer, PAGE_BUFFER_SIZE - 1);
    
    file_stream_close(app->file_stream);
    
    if(bytes_read == 0) {
        // No more content, wrap to beginning
        app->file_offset = 0;
        app->current_page_size = 0;
        widget_add_text_scroll_element(
            app->widget, 0, 0, WIDGET_WIDTH, WIDGET_HEIGHT, "End of document.");
        view_dispatcher_switch_to_view(app->view_dispatcher, WidgetView);
        return;
    }
    
    // Find a sensible break point
    size_t page_end = find_page_break(app->page_buffer, bytes_read);
    
    // Store the actual page size for navigation
    app->current_page_size = page_end;
    
    // Null-terminate at the break point
    app->page_buffer[page_end] = '\0';

    widget_add_text_scroll_element(
        app->widget, 0, 0, WIDGET_WIDTH, WIDGET_HEIGHT, app->page_buffer);

    view_dispatcher_switch_to_view(app->view_dispatcher, WidgetView);
}

bool topic_scene_on_event(void* context, SceneManagerEvent event) {
    App* app = context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NextPageEvent) {
            // Advance by the actual size of the current page
            app->file_offset += app->current_page_size;
            // Refresh the current scene instead of pushing a new one
            topic_scene_on_exit(app);
            topic_scene_on_enter(app);
            return true;
        }
        if(event.event == PrevPageEvent) {
            // Go back by the page size (approximate)
            if(app->file_offset > app->current_page_size) {
                app->file_offset -= app->current_page_size;
            } else {
                app->file_offset = 0;
            }
            // Refresh the current scene instead of pushing a new one
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
}