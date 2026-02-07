#pragma once
#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>
#include <vector>

class KeystrokeRecorder {
public:
    static bool StartRecording();

    static std::vector<CGKeyCode> StopRecording();

    static bool IsRecording();

    static void Clear();

private:
    static CGEventRef EventCallback(CGEventTapProxy proxy, CGEventType type,
                                    CGEventRef event, void* refcon);

    static std::vector<CGKeyCode> recordedKeys;
    static CFMachPortRef eventTap;
    static CFRunLoopSourceRef runLoopSource;
    static bool isRecording;
};
