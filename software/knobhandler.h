#ifndef KNOBHANDLER_H
#define KNOBHANDLER_H

#include <minwindef.h>
#endif // KNOBHANDLER_H

class KnobHandler{
public:
    //volume controls
    static void volumeUp();
    static void volumeDown();
    static void mute();

    //scrolling controls
    static void scrollUp();
    static void scrollDown();
    static void autoScrollToggle();


    // Brightness controls
    static void brightnessUp();
    static void brightnessDown();
    static void brightnessToggle();

    // App‐switcher controls
    static void activateAppSwitcher();
    static void switchAppRight();
    static void switchAppLeft();

    //Zoom In and Out controls
    static void zoomIn();
    static void zoomOut();
    static void zoomReset();

    //browser tab switching controls
    static void nextTab();
    static void previousTab();

private:
    // Low‐level key injection
    static void sendSingleKey(WORD key);
    static void sendKeyCombo(WORD modifier, WORD key);

    // Tracks whether Task View is open
    static bool appSwitcherActive;
};
