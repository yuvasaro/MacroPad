#include "keystrokerecorder.h"
#include <CoreGraphics/CGEventTypes.h>
#include <CoreGraphics/CGRemoteOperation.h>
#include <iostream>

std::vector<CGKeyCode> KeystrokeRecorder::recordedKeys;
CFMachPortRef KeystrokeRecorder::eventTap = nullptr;
CFRunLoopSourceRef KeystrokeRecorder::runLoopSource = nullptr;
bool KeystrokeRecorder::isRecording = false;

CGEventRef KeystrokeRecorder::EventCallback(CGEventTapProxy proxy, CGEventType type,
                                            CGEventRef event, void* refcon) {
    if (!isRecording) return event;

    if (type == kCGEventKeyDown || type == kCGEventFlagsChanged) {
        CGKeyCode keycode = (CGKeyCode)CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);
        recordedKeys.push_back(keycode);
        std::cout << "Recorded keycode: " << keycode << std::endl;
    }

    return event;
}

bool KeystrokeRecorder::StartRecording() {
    if (isRecording) return true;

    recordedKeys.clear();

    CGEventMask eventMask = CGEventMaskBit(kCGEventKeyDown) | CGEventMaskBit(kCGEventFlagsChanged);
    eventTap = CGEventTapCreate(
        kCGSessionEventTap,
        kCGHeadInsertEventTap,
        kCGEventTapOptionDefault,
        eventMask,
        EventCallback,
        nullptr
        );

    if (!eventTap) {
        std::cerr << "Failed to create event tap. Check Accessibility permissions." << std::endl;
        return false;
    }

    runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
    CGEventTapEnable(eventTap, true);

    isRecording = true;
    std::cout << "Recording started..." << std::endl;
    return true;
}

std::vector<CGKeyCode> KeystrokeRecorder::StopRecording() {
    if (!isRecording) return recordedKeys;

    CGEventTapEnable(eventTap, false);
    if (runLoopSource) {
        CFRunLoopRemoveSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
        CFRelease(runLoopSource);
        runLoopSource = nullptr;
    }
    if (eventTap) {
        CFRelease(eventTap);
        eventTap = nullptr;
    }

    isRecording = false;
    std::cout << "Recording stopped. Captured " << recordedKeys.size() << " keys." << std::endl;
    return recordedKeys;
}

bool KeystrokeRecorder::IsRecording() {
    return isRecording;
}

void KeystrokeRecorder::Clear() {
    recordedKeys.clear();
}
