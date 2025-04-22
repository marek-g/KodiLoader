# KodiLoader

This loader makes a few runtime changes to Kodi to make it more user friendly on desktop in window mode on Windows OS:
- starts Kodi in the portable mode (adds "-p" parameter to the executable)
- removes window frame
- makes the wole window draggable (and resizable through borders)
- double click toggles between fullscreen and window mode
- adds auto-hidden helper buttons in top left area of the window

Helper buttons include:
- "M" (mouse) - toggles between original mouse behavior and using mouse to move/resize the window
- "F" (fullscreen) - toggles between fullscreen and window mode
- "A" (audio) - toggles audio on/off
- "Q" (quit) - exits the app

## Usage

Copy files to the Kodi's folder. Run `KodiLoader32.exe` for 32-bit version of Kodi or `KodiLoader64.exe` for 64-bit version of Kodi.

## How it works

The loader uses `detours` library to inject custom dll to the Kodi's process. It replaces the original `RegisterClassExW` method with a custom one that overrides original `WndProc` and modifies window / handle custom window events from there.

## License

It's Public Domain.
