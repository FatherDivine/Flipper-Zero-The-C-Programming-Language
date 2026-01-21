#include "callbacks.h"
#include "../resource/resource.h"
#include "../scenes/scenes.h"
#include <stdlib.h>

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
        app->current_page = 0;
        app->total_pages = 0;
        app->file_size = 0;

        scene_manager_next_scene(app->scene_manager, TopicScene);
    }
}

bool back_event_callback(void* context) {
    furi_assert(context);
    App* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

// NEW: widget input callback for paging
bool widget_input_callback(InputEvent* event, void* context) {
    App* app = context;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyRight) {
            scene_manager_handle_custom_event(app->scene_manager, NextPageEvent);
            return true;
        }
        if(event->key == InputKeyLeft) {
            scene_manager_handle_custom_event(app->scene_manager, PrevPageEvent);
            return true;
        }
    }

    return false;
}