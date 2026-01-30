# Credits Scene Scrolling Fix

## Problem Description
The credits scene (lines 221-247 in `scenes/scenes.c`) had inconsistent scrolling behavior. Users reported that scrolling "doesn't work after a few attempts."

## Root Cause Analysis

### Original Implementation
The `widget_input_callback()` function in `callbacks/callbacks.c` was routing arrow key inputs based on the `swap_arrow_keys` user setting:

**Default mode (swap_arrow_keys=false)**:
- Left/Right keys → Trigger `NextPageEvent`/`PrevPageEvent` (consumed by callback)
- Up/Down keys → Pass through to widget for scrolling

**Swapped mode (swap_arrow_keys=true)**:
- Up/Down keys → Trigger `NextPageEvent`/`PrevPageEvent` (consumed by callback)
- Left/Right keys → Pass through to widget for scrolling

### Why This Caused Issues
1. The credits scene uses `widget_add_text_scroll_element()` which provides built-in scrolling
2. The credits scene's `on_event` handler returns `false` for all events (ignores page events)
3. In default mode, pressing Left/Right would trigger page events that were ignored
4. Users would try Left/Right thinking they should scroll, but nothing would happen
5. Only Up/Down keys worked in default mode, creating confusion

## Solution Implemented

Modified `widget_input_callback()` in `callbacks/callbacks.c` (lines 127-138) to:

1. Detect when the user is in the **Credits scene** using `scene_manager_get_current_scene()`
2. When in Credits scene, allow **all four arrow keys** (Up/Down/Left/Right) to pass through to the widget
3. The widget's built-in text scroll element then handles all arrow key inputs for scrolling
4. Other scenes (like Topic scene) continue to use the configured paging behavior

### Code Changes

```c
// Get current scene to determine input handling
uint32_t current_scene = scene_manager_get_current_scene(app->scene_manager);

// Credits scene: all arrow keys should scroll, not page
if(current_scene == CreditsScene) {
    // Let all arrow keys pass through to widget for scrolling
    if(event->key == InputKeyUp || event->key == InputKeyDown ||
       event->key == InputKeyLeft || event->key == InputKeyRight) {
        return false;  // Let widget handle scrolling
    }
}
```

## Testing Instructions

### Manual Testing on Flipper Zero

1. **Build and install the app** on your Flipper Zero device

2. **Navigate to Credits scene**:
   - Launch the C Programming Language app
   - Select "About / Credits" from the main menu

3. **Test all arrow keys**:
   - Press **Up** → Credits should scroll up
   - Press **Down** → Credits should scroll down
   - Press **Left** → Credits should scroll up
   - Press **Right** → Credits should scroll down

4. **Test with swapped navigation**:
   - Go back to main menu
   - Select "Options"
   - Toggle "Nav" setting to "Up/Down"
   - Return to Credits scene
   - Verify all arrow keys still work for scrolling

5. **Verify no regression in Topic scene**:
   - Navigate to "Read from Beginning" or any book content
   - In default mode (Nav: Left/Right):
     - Left should go to previous page
     - Right should go to next page
     - Up/Down should scroll within current page
   - In swapped mode (Nav: Up/Down):
     - Up should go to previous page
     - Down should go to next page
     - Left/Right should scroll within current page

### Expected Behavior After Fix

| Scene | Mode | Up | Down | Left | Right |
|-------|------|----|----|------|-------|
| **Credits** | Any | Scroll Up | Scroll Down | Scroll Up | Scroll Down |
| **Topic** | Default | Scroll Up | Scroll Down | Prev Page | Next Page |
| **Topic** | Swapped | Prev Page | Next Page | Scroll Up | Scroll Down |

## Technical Details

### Flipper Zero Widget Scrolling
- The Flipper firmware's widget module provides `widget_add_text_scroll_element()`
- This function creates a scrollable text view with built-in input handling
- The widget expects to receive arrow key events directly via the input callback
- When the input callback returns `false`, the event is passed to the widget
- When the input callback returns `true`, the event is consumed and not passed to the widget

### Scene-Specific Input Handling
- The fix uses `scene_manager_get_current_scene()` from the Flipper firmware API
- This allows different input behavior for different scenes
- Credits scene: optimized for scrolling only (no paging concept)
- Topic scene: optimized for paging with optional scrolling within pages

## Files Modified

1. **callbacks/callbacks.c** (lines 127-138)
   - Added scene detection logic
   - Added Credits scene-specific input handling
   - Preserved existing behavior for all other scenes

## Compatibility

- ✅ Compatible with Flipper Zero firmware API
- ✅ Uses standard scene_manager functions
- ✅ Maintains backward compatibility with user settings
- ✅ No changes to data structures or storage formats
- ✅ No impact on other scenes or functionality

## Related Code

- `scenes/scenes.c` lines 221-261: Credits scene implementation
- `callbacks/callbacks.c` lines 122-168: Widget input callback
- `scenes/scenes.h` lines 12: CreditsScene enum definition
- `constants/constants.h` lines 7-8: Widget dimensions

## References

- Flipper Zero firmware scene_manager API: https://github.com/flipperdevices/flipperzero-firmware/blob/dev/applications/services/gui/scene_manager.h
- Widget API documentation: https://github.com/flipperdevices/flipperzero-firmware/blob/dev/applications/services/gui/modules/widget.h
