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

// Find the last word boundary (space or newline) in a buffer
// Returns the index of the character after the boundary
static size_t find_last_word_boundary(const char* buffer, size_t length) {
    if(length == 0) return 0;
    
    // Start from the end and look backwards for a space or newline
    for(size_t i = length; i > 0; i--) {
        if(buffer[i - 1] == ' ' || buffer[i - 1] == '\n') {
            // Found a boundary, return position after the space/newline
            return i;
        }
    }
    
    // No word boundary found, use the full length
    return length;
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

    // Seek to current page offset
    stream_seek(app->file_stream, app->file_offset, StreamOffsetFromStart);

    // Read one page into fixed buffer
    size_t bytes_read =
        stream_read(app->file_stream, (uint8_t*)app->page_buffer, PAGE_BUFFER_SIZE - 1);
    
    file_stream_close(app->file_stream);
    
    // If we read something, find the last word boundary
    if(bytes_read > 0) {
        // Find the last complete word to avoid breaking mid-word
        size_t display_length = find_last_word_boundary(app->page_buffer, bytes_read);
        
        // Store how many bytes we're actually displaying
        app->page_bytes_displayed = display_length;
        
        // Null-terminate at the word boundary
        app->page_buffer[display_length] = '\0';
    } else {
        app->page_bytes_displayed = 0;
        app->page_buffer[0] = '\0';
    }

    widget_add_text_scroll_element(
        app->widget, 0, 0, WIDGET_WIDTH, WIDGET_HEIGHT, app->page_buffer);

    view_dispatcher_switch_to_view(app->view_dispatcher, WidgetView);
}

bool topic_scene_on_event(void* context, SceneManagerEvent event) {
    App* app = context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NextPageEvent) {
            // Advance by exactly how many bytes we displayed
            if(app->page_bytes_displayed > 0) {
                app->file_offset += app->page_bytes_displayed;
            }
            // Refresh the current scene instead of pushing a new one
            topic_scene_on_exit(app);
            topic_scene_on_enter(app);
            return true;
        }
        if(event.event == PrevPageEvent) {
            // Go back by approximately one page worth of content
            // We use page_bytes_displayed as an estimate, or PAGE_BUFFER_SIZE if not available
            size_t back_amount = app->page_bytes_displayed > 0 ? app->page_bytes_displayed : PAGE_BUFFER_SIZE;
            if(app->file_offset >= back_amount) {
                app->file_offset -= back_amount;
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