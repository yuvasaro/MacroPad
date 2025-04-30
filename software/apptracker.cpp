// AppTracker.cpp

#include "apptracker.h"
#include <windows.h>
#include <Psapi.h>              // for GetModuleFileNameExA
#include <QFileInfo>

#pragma comment(lib, "Psapi.lib")

static AppTracker *instance = nullptr;
static HWINEVENTHOOK winEventHook = nullptr;

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
