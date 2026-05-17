# MacroPad Project
UCSD HKN Project Team 2024-2025
The MacroPad project is a physical macro keyboard that interacts with a companion desktop app, allowing users to trigger customized hotkeys and launch executables across different applications. From designing the PCB and firmware to building the cross-platform desktop interface, we constructed both the hardware and software aspects of this project end-to-end.

[MacroPad Project Documentation.pdf](https://github.com/user-attachments/files/27936859/MacroPad.Project.Documentation.pdf)

## System Overview
On the physical aspect, the MacroPad is a palm-sized mechanical keyboard built around an ESP32 module, featuring 9 hot-swappable keys, 2 rotary encoders, and a small OLED screen on each key that displays context-aware icons. The board connects to a desktop app over USB, sending key and encoder events while receiving profile-switch commands in return.

The desktop app runs as a background system-tray agent on Windows and macOS, automatically detecting the active application and switching between six user-configurable profiles in real time. Each profile holds 9 macros — either keystroke combinations or executable launches — and is saved locally to the OS config directory. A QML-based GUI lets users drag-and-drop icons onto keys, configure macros, and preview what will appear on each OLED display.

<img src="https://github.com/user-attachments/assets/a738b8c8-45b8-41a9-91e3-906b14a6907a" alt="This is an illustration of how the MacroPad should be like" width="500" >

## Project Structure
Unlike typical macro pads that require manual profile switching, MacroPad automatically detects the active application on your computer and switches to the matching profile in real time. 

- **Macro class:** contains information about the type and content of each key.   
- **Profile class:** contains 9 macro keys.   
   - We can set up multiple profiles. profile objects are saved to the cross-platform config directory, making the app a background agent.     
   - Similar to how Logitech G Hub keeps running after the window is closed
- **UI:** A basic UI featuring the 9 key-grid for the general profile and 6 other profiles. 
   - Each key opens a configuration page where users can assign keystroke combination or an executable path which are saved to a file.
- **Hotkey Registering:** We can enter the key type and content as parameters, so it will be registering the information into a macro key in the current profile we are on.
- **Hotkey Triggering:** Everytime a hotkey is pressed, the registered executable or keystrokes will be triggered. We worked on translates a string into a list of keys that the computer can interpret. 
## Dependencies
Mac OS  
Windows 10

