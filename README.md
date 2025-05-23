# MacroPad Project
UCSD HKN Project Team 2024-2025
## Description
We aim to create a physical MacroPad that will interact with the app we created. The MacroPad contains 9 keys, each key can trigger certain events, such as keystrokes or opening executables, depending on which app the user is focused on. We are developing the software using Qt Creator which allows us to give dependency on both Windows and Mac OS. 

<img src="https://github.com/user-attachments/assets/a738b8c8-45b8-41a9-91e3-906b14a6907a" alt="This is an illustration of how the MacroPad should be like" width="500" >

## Project Structure

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

## Third-Party Dependencies
This project includes the [`brightness`](https://github.com/nriley/brightness) utility by Nicholas Riley, licensed under the BSD 2-Clause License.
