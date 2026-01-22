#include "app.h"
#include "../scenes/scenes.h"
#include "../scenes/scene_manager.h"
#include "../callbacks/callbacks.h"
#include <furi.h>
#include <string.h>

// Settings file format helper
static void ensure_settings_dir(void) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    storage_simply_mkdir(storage, APP_DATA_PATH(""));
    furi_record_close(RECORD_STORAGE);
}

App* app_alloc() {
    App* app = calloc(1, sizeof(App));
    app->file_stream = file_stream_alloc(furi_record_open(RECORD_STORAGE));
    if(!app->file_stream) {
        free(app);
        return NULL;
    }
    
    app->notifications = furi_record_open(RECORD_NOTIFICATION);
    
    app->scene_manager = scene_manager_alloc(&scene_manager_handlers, app);
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, back_event_callback);

    app->submenu = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, SubmenuView, submenu_get_view(app->submenu));

    app->widget = widget_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, WidgetView, widget_get_view(app->widget));

    // Set widget input callback for paging
    view_set_context(widget_get_view(app->widget), app);
    view_set_input_callback(widget_get_view(app->widget), widget_input_callback);

    // Initialize state
    app->backlight_on = false;
    app->backlight_timeout_sec = 0;  // 0 = always on when enabled
    app->swap_arrow_keys = false;
    app->has_reading_position = false;
    app->bookmark_count = 0;
    app->settings_loaded = false;
    
    // Load settings from file
    app_load_settings(app);

    return app;
}

void app_free(App* app) {
    furi_assert(app);
    
    // Save reading position before exit
    if(app->current_topic != NULL) {
        app_save_reading_position(app);
        app_save_settings(app);
    }
    
    // Turn off backlight lock if it was on
    if(app->backlight_on) {
        notification_message(app->notifications, &sequence_display_backlight_enforce_auto);
    }
    
    furi_record_close(RECORD_NOTIFICATION);
    
    view_dispatcher_remove_view(app->view_dispatcher, SubmenuView);
    view_dispatcher_remove_view(app->view_dispatcher, WidgetView);
    scene_manager_free(app->scene_manager);
    view_dispatcher_free(app->view_dispatcher);
    submenu_free(app->submenu);
    widget_free(app->widget);
    stream_free(app->file_stream);
    free(app);
}

void app_load_settings(App* app) {
    ensure_settings_dir();
    
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    
    if(storage_file_open(file, SETTINGS_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        char line[256];
        size_t bytes_read;
        size_t line_pos = 0;
        
        while((bytes_read = storage_file_read(file, line + line_pos, 1)) > 0) {
            if(line[line_pos] == '\n' || line_pos >= 254) {
                line[line_pos] = '\0';
                
                // Parse settings
                if(strncmp(line, "backlight=", 10) == 0) {
                    app->backlight_on = (line[10] == '1');
                } else if(strncmp(line, "backlight_timeout=", 18) == 0) {
                    app->backlight_timeout_sec = (uint32_t)strtoul(line + 18, NULL, 10);
                } else if(strncmp(line, "swap_arrow_keys=", 16) == 0) {
                    app->swap_arrow_keys = (line[16] == '1');
                } else if(strncmp(line, "last_topic=", 11) == 0) {
                    strncpy(app->last_topic_path, line + 11, sizeof(app->last_topic_path) - 1);
                    app->last_topic_path[sizeof(app->last_topic_path) - 1] = '\0';
                    app->has_reading_position = (strlen(app->last_topic_path) > 0);
                } else if(strncmp(line, "last_offset=", 12) == 0) {
                    app->last_file_offset = (size_t)strtoul(line + 12, NULL, 10);
                } else if(strncmp(line, "last_chapter=", 13) == 0) {
                    app->last_chapter_index = (size_t)strtoul(line + 13, NULL, 10);
                } else if(strncmp(line, "last_topic_idx=", 15) == 0) {
                    app->last_topic_index = (size_t)strtoul(line + 15, NULL, 10);
                } else if(strncmp(line, "bookmark=", 9) == 0) {
                    // Format: bookmark=topic_path|offset
                    if(app->bookmark_count < MAX_BOOKMARKS) {
                        char* sep = strchr(line + 9, '|');
                        if(sep) {
                            *sep = '\0';
                            strncpy(app->bookmark_topics[app->bookmark_count], line + 9, 127);
                            app->bookmark_topics[app->bookmark_count][127] = '\0';
                            app->bookmark_offsets[app->bookmark_count] = (size_t)strtoul(sep + 1, NULL, 10);
                            app->bookmark_count++;
                        }
                    }
                }
                
                line_pos = 0;
            } else {
                line_pos++;
            }
        }
        
        app->settings_loaded = true;
    }
    
    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

void app_save_settings(App* app) {
    ensure_settings_dir();
    
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    
    if(storage_file_open(file, SETTINGS_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        char line[256];
        
        // Save backlight setting
        snprintf(line, sizeof(line), "backlight=%d\n", app->backlight_on ? 1 : 0);
        storage_file_write(file, line, strlen(line));
        
        // Save backlight timeout
        snprintf(line, sizeof(line), "backlight_timeout=%u\n", app->backlight_timeout_sec);
        storage_file_write(file, line, strlen(line));
        
        // Save swap arrow keys setting
        snprintf(line, sizeof(line), "swap_arrow_keys=%d\n", app->swap_arrow_keys ? 1 : 0);
        storage_file_write(file, line, strlen(line));
        
        // Save last reading position
        if(app->has_reading_position && strlen(app->last_topic_path) > 0) {
            snprintf(line, sizeof(line), "last_topic=%s\n", app->last_topic_path);
            storage_file_write(file, line, strlen(line));
            
            snprintf(line, sizeof(line), "last_offset=%zu\n", app->last_file_offset);
            storage_file_write(file, line, strlen(line));
            
            snprintf(line, sizeof(line), "last_chapter=%zu\n", app->last_chapter_index);
            storage_file_write(file, line, strlen(line));
            
            snprintf(line, sizeof(line), "last_topic_idx=%zu\n", app->last_topic_index);
            storage_file_write(file, line, strlen(line));
        }
        
        // Save bookmarks
        for(size_t i = 0; i < app->bookmark_count; i++) {
            snprintf(line, sizeof(line), "bookmark=%s|%zu\n", 
                    app->bookmark_topics[i], app->bookmark_offsets[i]);
            storage_file_write(file, line, strlen(line));
        }
    }
    
    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

void app_save_reading_position(App* app) {
    if(app->current_topic != NULL) {
        strncpy(app->last_topic_path, app->current_topic, sizeof(app->last_topic_path) - 1);
        app->last_topic_path[sizeof(app->last_topic_path) - 1] = '\0';
        app->last_file_offset = app->file_offset;
        app->last_chapter_index = app->current_chapter_index;
        app->last_topic_index = app->chapter_selected_index;
        app->has_reading_position = true;
    }
}

bool app_check_sd_card(App* app) {
    UNUSED(app);
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FS_Error status = storage_sd_status(storage);
    furi_record_close(RECORD_STORAGE);
    return (status == FSE_OK);
}

void app_add_bookmark(App* app) {
    if(app->current_topic == NULL || app->bookmark_count >= MAX_BOOKMARKS) {
        return;
    }
    
    // Check if bookmark already exists
    for(size_t i = 0; i < app->bookmark_count; i++) {
        if(strcmp(app->bookmark_topics[i], app->current_topic) == 0 &&
           app->bookmark_offsets[i] == app->file_offset) {
            return; // Already bookmarked
        }
    }
    
    // Add new bookmark
    strncpy(app->bookmark_topics[app->bookmark_count], app->current_topic, 127);
    app->bookmark_topics[app->bookmark_count][127] = '\0';
    app->bookmark_offsets[app->bookmark_count] = app->file_offset;
    app->bookmark_count++;
    app->current_page_bookmarked = true;
    
    // Save immediately
    app_save_settings(app);
    
    // Notify user
    notification_message(app->notifications, &sequence_success);
}

void app_remove_bookmark(App* app) {
    if(app->current_topic == NULL) return;
    
    for(size_t i = 0; i < app->bookmark_count; i++) {
        if(strcmp(app->bookmark_topics[i], app->current_topic) == 0 &&
           app->bookmark_offsets[i] == app->file_offset) {
            // Remove by shifting remaining bookmarks
            for(size_t j = i; j < app->bookmark_count - 1; j++) {
                strcpy(app->bookmark_topics[j], app->bookmark_topics[j + 1]);
                app->bookmark_offsets[j] = app->bookmark_offsets[j + 1];
            }
            app->bookmark_count--;
            app->current_page_bookmarked = false;
            app_save_settings(app);
            return;
        }
    }
}

bool app_is_page_bookmarked(App* app) {
    if(app->current_topic == NULL) return false;
    
    for(size_t i = 0; i < app->bookmark_count; i++) {
        if(strcmp(app->bookmark_topics[i], app->current_topic) == 0 &&
           app->bookmark_offsets[i] == app->file_offset) {
            return true;
        }
    }
    return false;
}

void app_set_backlight(App* app, bool on) {
    app->backlight_on = on;
    if(on) {
        // When backlight is enabled, enforce it on
        // The timeout setting can be used in future enhancements
        // For now, we keep it simple: ON = always on while in app
        notification_message(app->notifications, &sequence_display_backlight_enforce_on);
    } else {
        // When backlight is disabled, restore automatic system control
        notification_message(app->notifications, &sequence_display_backlight_enforce_auto);
    }
}