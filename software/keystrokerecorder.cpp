#include "keystrokerecorder.h"
#include <CoreGraphics/CGEventTypes.h>
#include <CoreGraphics/CGRemoteOperation.h>
#include <algorithm>
#include <iostream>

std::vector<CGKeyCode> KeystrokeRecorder::recordedKeys;
CFMachPortRef KeystrokeRecorder::eventTap = nullptr;
CFRunLoopSourceRef KeystrokeRecorder::runLoopSource = nullptr;
bool KeystrokeRecorder::isRecording = false;

CGEventRef KeystrokeRecorder::EventCallback(CGEventTapProxy proxy, CGEventType type,
                                            CGEventRef event, void* refcon) {
    if (!isRecording) return event;

    if (type == kCGEventTapDisabledByTimeout || type == kCGEventTapDisabledByUserInput) {
        CGEventTapEnable(eventTap, true);
        return nullptr;
    }

    auto appendKeyIfMissing = [](CGKeyCode recordedKey) {
        if (std::find(recordedKeys.begin(), recordedKeys.end(), recordedKey) == recordedKeys.end()) {
            recordedKeys.push_back(recordedKey);
        }
    };

    if (type == kCGEventKeyDown || type == kCGEventFlagsChanged) {
        CGKeyCode keycode = (CGKeyCode)CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);
        appendKeyIfMissing(keycode);

        std::cout << "Recorded keys down:";
        for (CGKeyCode recordedKey : recordedKeys) {
            std::cout << " " << recordedKey;
        }
        std::cout << std::endl;
    }

    if (type == kCGEventKeyDown || type == kCGEventKeyUp || type == kCGEventFlagsChanged) {
        return nullptr;
    }

    return event;
}

bool KeystrokeRecorder::StartRecording() {
    if (isRecording) return true;

    if (!AXIsProcessTrusted()) {
        std::cerr << "Not trusted for Accessibility. Grant permission to the app that launches this binary.\n";
    }

    if (!CGPreflightListenEventAccess()) {
        std::cerr << "Input Monitoring permission is required to capture and block system shortcuts while recording.\n";
        CGRequestListenEventAccess();
        return false;
    }

    recordedKeys.clear();

    CGEventMask eventMask = CGEventMaskBit(kCGEventKeyDown)
                           | CGEventMaskBit(kCGEventKeyUp)
                           | CGEventMaskBit(kCGEventFlagsChanged);
    eventTap = CGEventTapCreate(
        kCGHIDEventTap,
        kCGHeadInsertEventTap,
        kCGEventTapOptionDefault,
        eventMask,
        EventCallback,
        nullptr
        );

    if (!eventTap) {
        std::cerr << "Failed to create event tap. Check Accessibility and Input Monitoring permissions." << std::endl;
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
