#include "callbacks.h"
#include "../resource/resource.h"
#include "../scenes/scenes.h"
#include <stdlib.h>
#include <furi.h>

// Start menu callback
void start_menu_callback(void* context, uint32_t index) {
    App* app = context;
    
    switch(index) {
    case StartMenuContinue:
        // Continue reading from last saved position
        if(app->has_reading_position && strlen(app->last_topic_path) > 0) {
            app->current_topic = app->last_topic_path;
            app->file_offset = app->last_file_offset;
            app->current_chapter_index = app->last_chapter_index;
            app->chapter_selected_index = app->last_topic_index;
            app->total_pages = 0; // Will be recalculated
            app->current_page = 0;
            scene_manager_next_scene(app->scene_manager, TopicScene);
        }
        break;
    case StartMenuRead:
        // Start reading from the very beginning (Preface)
        if(number_of_chapters > 0 && chapters[0].number_of_topics > 0) {
            app->current_chapter_index = 0;
            app->chapter_selected_index = 0;
            app->current_topic = chapters[0].content[0].file_path;
            app->file_offset = 0;
            app->total_pages = 0;
            app->current_page = 0;
            scene_manager_next_scene(app->scene_manager, TopicScene);
        }
        break;
    case StartMenuTOC:
        scene_manager_next_scene(app->scene_manager, MainMenuScene);
        break;
    case StartMenuBookmarks:
        scene_manager_next_scene(app->scene_manager, BookmarksScene);
        break;
    case StartMenuOptions:
        scene_manager_next_scene(app->scene_manager, OptionsScene);
        break;
    case StartMenuCredits:
        scene_manager_next_scene(app->scene_manager, CreditsScene);
        break;
    }
}

void menu_callback(void* context, uint32_t index) {
    App* app = context;

    // Remember selection
    app->main_menu_selected_index = index;

    if(index < number_of_chapters) {
        app->current_chapter_index = index;
        scene_manager_next_scene(app->scene_manager, ChapterScene);
    }
}

void chapter_callback(void* context, uint32_t index) {
    App* app = (App*)context;

    // Remember selection
    app->chapter_selected_index = index;

    size_t chapterIndex = app->current_chapter_index;
    Chapter currentChapter = chapters[chapterIndex];

    if(index < currentChapter.number_of_topics) {
        app->current_topic = currentChapter.content[index].file_path;

        // Reset paging when opening a new topic
        app->file_offset = 0;
        app->current_page_size = 0;
        app->total_pages = 0;
        app->current_page = 0;

        scene_manager_next_scene(app->scene_manager, TopicScene);
    }
}

// Options callback
void options_callback(void* context, uint32_t index) {
    App* app = context;
    
    // Remember selection
    app->options_selected_index = index;
    
    scene_manager_handle_custom_event(app->scene_manager, index);
}

// Bookmarks callback
void bookmarks_callback(void* context, uint32_t index) {
    App* app = context;
    
    // Remember selection
    app->bookmarks_selected_index = index;
    
    if(index < app->bookmark_count) {
        // Navigate to bookmarked position
        app->current_topic = app->bookmark_topics[index];
        app->file_offset = app->bookmark_offsets[index];
        app->total_pages = 0; // Will be recalculated
        app->current_page = 0;
        scene_manager_next_scene(app->scene_manager, TopicScene);
    }
}

bool back_event_callback(void* context) {
    furi_assert(context);
    App* app = context;
    
    // Save settings when going back
    app_save_settings(app);
    
    return scene_manager_handle_back_event(app->scene_manager);
}

// Widget input callback for paging and bookmarking
bool widget_input_callback(InputEvent* event, void* context) {
    App* app = context;
    static uint32_t ok_press_start = 0;

    if(event->type == InputTypeShort || event->type == InputTypeRepeat) {
        // Determine which keys to use for paging based on swap_arrow_keys setting
        InputKey next_key = app->swap_arrow_keys ? InputKeyDown : InputKeyRight;
        InputKey prev_key = app->swap_arrow_keys ? InputKeyUp : InputKeyLeft;
        
        if(event->key == next_key) {
            scene_manager_handle_custom_event(app->scene_manager, NextPageEvent);
            return true;
        }
        if(event->key == prev_key) {
            scene_manager_handle_custom_event(app->scene_manager, PrevPageEvent);
            return true;
        }
        // When swap is enabled, let Left/Right pass through for widget scroll
        // When swap is disabled, let Up/Down pass through for widget scroll
        InputKey scroll_key1 = app->swap_arrow_keys ? InputKeyLeft : InputKeyUp;
        InputKey scroll_key2 = app->swap_arrow_keys ? InputKeyRight : InputKeyDown;
        if(event->key == scroll_key1 || event->key == scroll_key2) {
            return false;  // Let widget handle scrolling
        }
    }
    
    // Handle long press of OK button for bookmark (1+ second)
    if(event->key == InputKeyOk) {
        if(event->type == InputTypePress) {
            ok_press_start = furi_get_tick();
        } else if(event->type == InputTypeRelease) {
            uint32_t press_duration = furi_get_tick() - ok_press_start;
            // Check if held for more than 1 second (1000ms = 1000 ticks at 1kHz)
            if(press_duration >= 1000) {
                scene_manager_handle_custom_event(app->scene_manager, BookmarkEvent);
                return true;
            }
        } else if(event->type == InputTypeLong) {
            // Also handle InputTypeLong as alternative
            scene_manager_handle_custom_event(app->scene_manager, BookmarkEvent);
            return true;
        }
    }

    return false;
}