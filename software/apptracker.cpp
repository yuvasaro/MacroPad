// AppTracker.cpp

#include "AppTracker.h"
#include <windows.h>
#include <Psapi.h>              // for GetModuleFileNameExA
#include <QFileInfo>

#pragma comment(lib, "Psapi.lib")

static AppTracker *instance = nullptr;
static HWINEVENTHOOK winEventHook = nullptr;

void CALLBACK handleWinEvent(HWINEVENTHOOK, DWORD, HWND hwnd, LONG, LONG, DWORD, DWORD) {
    if (!instance || !hwnd) return;

    // 1) Get the PID for this window
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == 0) return;

    // 2) Open the process with query rights
    HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hProc) return;

    // 3) Query the full image name
    char exePath[MAX_PATH];
    DWORD size = MAX_PATH;
    BOOL ok = QueryFullProcessImageNameA(hProc, 0, exePath, &size);
    CloseHandle(hProc);
    if (!ok) return;

    // 4) Extract just the base name (no path, no extension)
    QString appName = QFileInfo(QString::fromLocal8Bit(exePath)).completeBaseName();
    if (!appName.isEmpty()) {
        emit instance->appChanged(appName);
    }
}

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
