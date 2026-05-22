#ifndef KNOBHANDLER_H
#define KNOBHANDLER_H

#ifdef _WIN32
#include <minwindef.h>
#endif
#ifdef __APPLE__
#include <Carbon/Carbon.h>
#include <ApplicationServices/ApplicationServices.h>
#endif


class KnobHandler{
public:
    // Store mac volume
#ifdef __APPLE__
    static int macVolume;
    static int getSystemVolume();
    static bool setSystemVolume(int volume);
#endif

    // Volume controls
    static void volumeUp();
    static void volumeDown();
    static void toggleMute();

    // Scrolling controls
    static void scrollUp();
    static void scrollDown();
    static void autoScrollToggle();

    // Brightness controls
    static void brightnessUp();
    static void brightnessDown();
    static void brightnessToggle();

    // App switcher controls
    static void activateAppSwitcher();
    static void switchAppRight();
    static void switchAppLeft();

    // Zoom In and Out controls
    static void zoomIn();
    static void zoomOut();
    static void zoomReset();

    // Browser tab switching controls
    static void nextTab();
    static void previousTab();

    // App volume
    static void appVolumeUp();
    static void appVolumeDown();

private:
#ifdef _WIN32
    // Low‐level key injection
    static void sendSingleKey(WORD key);
    static void sendKeyCombo(WORD modifier, WORD key);
#endif
#ifdef __APPLE__
    static void postMacKey(CGKeyCode keyCode, bool keyDown, CGEventFlags flags);
    static void pressMacAppSwitcherKey(CGKeyCode keyCode, CGEventFlags flags);
    static void releaseMacAppSwitcher();
#endif

    // Tracks whether Task View is open
    static bool appSwitcherActive;
};

#endif // KNOBHANDLER_H
