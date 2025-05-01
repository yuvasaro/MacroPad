#include "AppTracker.h"
#include <windows.h>
#include <psapi.h>  // for GetModuleBaseName
#pragma comment(lib, "psapi.lib")  // Link the PSAPI lib


static AppTracker *instance = nullptr;  // Static instance for callback
static HWINEVENTHOOK winEventHook = nullptr;  // Windows event hook handle


#include <psapi.h>
#pragma comment(lib, "psapi.lib")

void CALLBACK handleWinEvent(HWINEVENTHOOK, DWORD, HWND hwnd, LONG, LONG, DWORD, DWORD) {
    if (instance && hwnd) {
        // Step 1: Get the top-level owner (safer than GA_ROOT alone)
        HWND topLevelHwnd = GetAncestor(hwnd, GA_ROOTOWNER);

        // Step 2: Make sure it's visible and has a title
        char title[256];
        GetWindowTextA(topLevelHwnd, title, sizeof(title));
        if (!IsWindowVisible(topLevelHwnd) || strlen(title) == 0) {
            return; // skip windows like overlays, taskbar buttons, etc.
        }

        // Step 3: Get process ID from this top-level window
        DWORD processId = 0;
        GetWindowThreadProcessId(topLevelHwnd, &processId);

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, processId);
        if (hProcess) {
            char processName[MAX_PATH] = {0};
            if (GetModuleBaseNameA(hProcess, NULL, processName, sizeof(processName))) {
                QString baseName = QString::fromLocal8Bit(processName).trimmed();
                if (baseName.endsWith(".exe", Qt::CaseInsensitive)) {
                    baseName.chop(4);
                }

                if (!baseName.isEmpty()) {
                    emit instance->appChanged(baseName);
                }
            }
            CloseHandle(hProcess);
        }
    }
}


/*
void CALLBACK handleWinEvent(HWINEVENTHOOK, DWORD, HWND hwnd, LONG, LONG, DWORD, DWORD) {
    if (instance && hwnd) {
        char windowTitle[256];
        GetWindowTextA(hwnd, windowTitle, sizeof(windowTitle));

        QString appName = QString::fromLocal8Bit(windowTitle).trimmed();
        if (!appName.isEmpty()) {
            emit instance->appChanged(appName);
        }
    }
}
*/
AppTracker::AppTracker(QObject *parent) : QObject(parent) {
    instance = this;
    startTracking();
}

AppTracker::~AppTracker() {
    stopTracking();
}

void AppTracker::startTracking() {
    winEventHook = SetWinEventHook(
        EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND,
        NULL,
        handleWinEvent,
        0, 0,
        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS
        );
}

void AppTracker::stopTracking() {
    if (winEventHook) {
        UnhookWinEvent(winEventHook);
        winEventHook = nullptr;
    }
}
