// AppTracker.cpp

#include "AppTracker.h"
#include <windows.h>
<<<<<<< HEAD
#include <psapi.h>  // for GetModuleBaseName
#pragma comment(lib, "psapi.lib")  // Link the PSAPI lib

=======
#include <Psapi.h>              // for GetModuleFileNameExA
#include <QFileInfo>
>>>>>>> e63944b1453586822d78afc97d753fe6a74c2d43

#pragma comment(lib, "Psapi.lib")

static AppTracker *instance = nullptr;
static HWINEVENTHOOK winEventHook = nullptr;


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
    if (!instance || !hwnd) return;

    // 1) only track visible windows with a caption bar
    if (!IsWindowVisible(hwnd))                return;
    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    if ((style & WS_CAPTION) == 0)             return;

    // 2) get the process ID
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == 0)                              return;

    // 3) open the process
    HANDLE hProc = OpenProcess(
        PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ,
        FALSE, pid
        );
    if (!hProc)                                return;

    // 4) get the full path and close handle
    char exePath[MAX_PATH];
    DWORD size = MAX_PATH;
    BOOL ok = QueryFullProcessImageNameA(hProc, 0, exePath, &size);
    CloseHandle(hProc);
    if (!ok)                                   return;

    // 5) skip explorer.exe
    QString fullName = QFileInfo(QString::fromLocal8Bit(exePath)).fileName();  // e.g. "notepad.exe"
    if (fullName.compare("explorer.exe", Qt::CaseInsensitive) == 0)
        return;

    // 6) emit just the base name
    QString appName = QFileInfo(QString::fromLocal8Bit(exePath)).completeBaseName();
    if (!appName.isEmpty()) {
        emit instance->appChanged(appName);
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
