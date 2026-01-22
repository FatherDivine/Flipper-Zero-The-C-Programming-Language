# The C Programming Language - Flipper Zero Edition

## Features

### Main Menu
The app starts with a main menu offering the following options:

1. **Continue Reading** (shown only when a saved position exists)
   - Resumes reading from the last page viewed before closing the app or rebooting the Flipper
   - Automatically hidden for first-time users

2. **Read from Beginning**
   - Starts reading from the very first topic (Preface)
   - Automatically advances through all chapters in sequence

3. **Table of Contents**
   - Browse chapters and topics
   - Select any specific section to read

4. **Bookmarks**
   - View all saved bookmarks
   - Navigate directly to any bookmarked page

5. **Options**
   - Configure app settings (backlight control)

6. **About / Credits**
   - View credits and version information

### Reading Features

#### Page Navigation
- **Right button** (or Down if swapped): Go to next page
- **Left button** (or Up if swapped): Go to previous page
- **Back button**: Return to previous menu
- Navigation controls can be swapped in Options menu

#### Page Display
- 4 lines of content per page for optimal readability
- Footer line shows: `< [current page/total pages] >`
- Proper word wrapping at line boundaries
- No text skipping between pages

#### Bookmarks
- **Hold OK button for 1+ second**: Toggle bookmark on current page
- Bookmarked pages show an asterisk (*) in the footer: `< * [page/total] >`
- All bookmarks are saved to settings file and persist across app restarts

### Settings

Settings are automatically saved to `app_data/c_book/settings.config` and include:

- **Backlight Setting**: Toggle between "ON" (backlight enabled) and "OFF" (use system default)
- **Backlight Timeout**: Choose timeout duration (Always On, 10s, 30s, 60s, 90s)
- **Navigation Controls**: Choose between Left/Right (default) or Up/Down for page navigation
- **Last Reading Position**: Automatically saved when exiting a topic or the app
- **Bookmarks**: All bookmark positions and their associated topics

### Backlight Control

- When **Backlight: ON** is selected in Options:
  - Screen backlight stays on while in the app
  - Automatically returns to normal behavior when app exits
  
- When **Backlight: OFF** is selected:
  - Normal Flipper backlight timeout behavior
  - Screen will turn off after inactivity

- **Timeout Setting**: Configures the desired timeout
  - (0=always on, 10s, 30s, 60s, 90s)

### Navigation Controls

- **Nav: Left/Right** (default):
  - Right button: Go to next page
  - Left button: Go to previous page
  - Up/Down buttons: Scroll within pages (e.g., Credits scene)

- **Nav: Up/Down** (when swapped):
  - Down button: Go to next page
  - Up button: Go to previous page
  - Left/Right buttons: Scroll within pages (e.g., Credits scene)

### Error Handling

#### SD Card Detection
- The app checks for SD card availability before accessing files
- If SD card is removed or unavailable:
  - Displays a friendly error message
  - Does not freeze or crash
  - Allows user to navigate back to menu

#### File Access Errors
- Graceful handling of missing or corrupted files
- Clear error messages displayed to user

### Technical Details

- **Settings File Location**: `/ext/apps_data/c_book/settings.config`
- **Maximum Bookmarks**: 64
- **Maximum Pages per Topic**: 512
- **Display Width**: 25 characters per line
- **Display Height**: 4 content lines + 1 footer line

## Version History

### v0.4 (In Development)
- Fixed Credits scene scrolling (up/down keys now work for scrolling)
- Added navigation control swap option (Left/Right vs Up/Down)
- Added backlight timeout configuration (Always On, 10s, 30s, 60s, 90s)
- Enhanced Options menu with three configurable settings
- Improved backlight control documentation

### v0.3
- Added main menu system (Read, Continue Reading, Table of Contents, etc.)
- Added bookmark feature with center button hold
- Added settings persistence
- Added backlight control
- Added SD card error handling
- Fixed pagination to prevent text skipping
- Added page number display with navigation hints
- Improved word wrapping

### Previous Versions
- v0.2: Original implementation by @armixz
- v0.1: Initial concept

## Credits

- **Original Idea & Implementation**: @armixz
  - Preface content
  - Chapter 1 content
  - Initial app concept

- **Completion & Polish**: @FatherDivine
  - Additional chapters
  - Menu system
  - Bookmarks
  - Settings persistence
  - Pagination fixes
  - Error handling
  - Documentation
