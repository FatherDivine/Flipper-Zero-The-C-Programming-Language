#ifndef APP_H
#define APP_H

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/widget.h>
#include <gui/modules/submenu.h>
#include <gui/modules/text_input.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>

#include <gui/scene_manager.h>
#include <storage/filesystem_api_defines.h>
#include <storage/storage.h>
#include <toolbox/stream/stream.h>
#include <toolbox/stream/file_stream.h>

#define PAGE_BUFFER_SIZE 1024
#define MAX_PAGE_HISTORY 512
#define DISPLAY_BUFFER_SIZE (PAGE_BUFFER_SIZE + 128)
#define MAX_BOOKMARKS 64
#define SETTINGS_PATH APP_DATA_PATH("settings.config")

typedef struct App {
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;
    Submenu* submenu;
    Widget* widget;
    const char* current_topic;
    size_t current_chapter_index;
    Stream* file_stream;
    NotificationApp* notifications;

    // Menu selection memory
    uint32_t main_menu_selected_index;
    uint32_t chapter_selected_index;
    uint32_t bookmarks_selected_index;
    uint32_t options_selected_index;

    // Paging state
    size_t file_offset;
    size_t current_page_size; // Number of bytes displayed on current page
    char page_buffer[PAGE_BUFFER_SIZE];
    size_t page_bytes_displayed; // Track actual bytes displayed on current page

    // Page tracking for proper pagination
    size_t file_size;
    size_t total_pages;
    size_t current_page;
    size_t page_offsets[MAX_PAGE_HISTORY];
    char display_buffer[DISPLAY_BUFFER_SIZE];

    // Continue reading state
    bool has_reading_position;
    char last_topic_path[128];
    size_t last_file_offset;
    size_t last_chapter_index;
    size_t last_topic_index;

    // Bookmarks
    size_t bookmark_offsets[MAX_BOOKMARKS];
    char bookmark_topics[MAX_BOOKMARKS][128];
    size_t bookmark_count;
    bool current_page_bookmarked;

    // Settings
    bool backlight_on;
    uint32_t backlight_timeout_sec;  // Backlight timeout in seconds (0 = always on)
    bool swap_arrow_keys;  // Swap left/right with up/down
    bool settings_loaded;

} App;

App* app_alloc();
void app_free(App* app);

// Settings functions
void app_load_settings(App* app);
void app_save_settings(App* app);
void app_save_reading_position(App* app);
bool app_check_sd_card(App* app);

// Bookmark functions
void app_add_bookmark(App* app);
void app_remove_bookmark(App* app);
bool app_is_page_bookmarked(App* app);

// Backlight control
void app_set_backlight(App* app, bool on);

#endif // APP_H