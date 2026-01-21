#ifndef APP_H
#define APP_H

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/widget.h>
#include <gui/modules/submenu.h>
#include <gui/modules/text_input.h>

#include <gui/scene_manager.h>
#include <storage/filesystem_api_defines.h>
#include <storage/storage.h>
#include <toolbox/stream/stream.h>
#include <toolbox/stream/file_stream.h>
#include "../resource/resource.h"

#define PAGE_BUFFER_SIZE 1024
#define MAX_PAGE_HISTORY 128
#define DISPLAY_BUFFER_SIZE (PAGE_BUFFER_SIZE + 64)

typedef struct App {
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;
    Submenu* submenu;
    Widget* widget;
    const char* current_topic;
    size_t current_chapter_index;
    Stream* file_stream;

    // Menu selection memory
    uint32_t main_menu_selected_index;
    uint32_t chapter_selected_index;

    // Paging state
    size_t file_offset;
    size_t current_page_size; // Number of bytes displayed on current page
    char page_buffer[PAGE_BUFFER_SIZE];
    size_t page_bytes_displayed; // Track actual bytes displayed on current page

} App;

App* app_alloc();
void app_free(App* app);

#endif // APP_H