#ifndef SCENES_H
#define SCENES_H

#include "../app/app.h"

typedef enum {
    StartMenuScene,
    MainMenuScene,  // Table of Contents
    ChapterScene,
    TopicScene,
    OptionsScene,
    CreditsScene,
    BookmarksScene,
    SceneCount,
} MainScene;

typedef enum {
    SubmenuView,
    WidgetView,
} MainView;

typedef enum {
    MainMenuEvent,
    NextPageEvent,
    PrevPageEvent,
    BookmarkEvent,
    // Start menu events
    StartMenuContinue,
    StartMenuRead,
    StartMenuTOC,
    StartMenuBookmarks,
    StartMenuOptions,
    StartMenuCredits,
    // Options events
    OptionsBacklight,
} MainEvent;

// Start menu scene
void start_menu_scene_on_enter(void* context);
bool start_menu_scene_on_event(void* context, SceneManagerEvent event);
void start_menu_scene_on_exit(void* context);

// Table of Contents scene (main menu)
void main_menu_scene_on_enter(void* context);
bool main_menu_scene_on_event(void* context, SceneManagerEvent event);
void main_menu_scene_on_exit(void* context);

void chapter_scene_on_enter(void* context);
bool chapter_scene_on_event(void* context, SceneManagerEvent event);
void chapter_scene_on_exit(void* context);

void topic_scene_on_enter(void* context);
bool topic_scene_on_event(void* context, SceneManagerEvent event);
void topic_scene_on_exit(void* context);

// Options scene
void options_scene_on_enter(void* context);
bool options_scene_on_event(void* context, SceneManagerEvent event);
void options_scene_on_exit(void* context);

// Credits scene
void credits_scene_on_enter(void* context);
bool credits_scene_on_event(void* context, SceneManagerEvent event);
void credits_scene_on_exit(void* context);

// Bookmarks scene
void bookmarks_scene_on_enter(void* context);
bool bookmarks_scene_on_event(void* context, SceneManagerEvent event);
void bookmarks_scene_on_exit(void* context);

#endif // SCENES_H