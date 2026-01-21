#ifndef CALLBACKS_H
#define CALLBACKS_H

#include "../app/app.h"
#include <string.h>

// Start menu callback
void start_menu_callback(void* context, uint32_t index);

void menu_callback(void* context, uint32_t index);
void chapter_callback(void* context, uint32_t index);
bool back_event_callback(void* context);

// Options and bookmarks callbacks
void options_callback(void* context, uint32_t index);
void bookmarks_callback(void* context, uint32_t index);

// Widget input callback for paging and bookmarks
bool widget_input_callback(InputEvent* event, void* context);

#endif // CALLBACKS_H