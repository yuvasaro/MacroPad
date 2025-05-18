// AppTracker.cpp

#include "apptracker.h"
#include <windows.h>
#include <psapi.h>  // for GetModuleBaseName
#pragma comment(lib, "psapi.lib")  // Link the PSAPI lib
#include <Psapi.h>              // for GetModuleFileNameExA
#include <QFileInfo>
#include <QDebug>


static AppTracker *instance = nullptr;
static HWINEVENTHOOK winEventHook = nullptr;

// this callback runs on every foreground-change event
void CALLBACK handleWinEvent(HWINEVENTHOOK, DWORD, HWND hwnd, LONG, LONG, DWORD, DWORD) {
    if (!instance) {
        return;
    }
    if (!hwnd) {
        return;
    }

    // Step 1: Get the top-level owner
    HWND topLevelHwnd = GetAncestor(hwnd, GA_ROOTOWNER);
    // Step 2: Title check
    char title[256] = {0};
    int len = GetWindowTextA(topLevelHwnd, title, sizeof(title));
    if (!IsWindowVisible(topLevelHwnd) || len == 0) {
        qDebug() << "[AppTracker] Skipping invisible or untitled window.";
        return;
    }

    // Step 3: Get process ID
    DWORD processId = 0;
    GetWindowThreadProcessId(topLevelHwnd, &processId);
    if (processId == 0) {
        return;
    }

    // Step 4: Open process
    HANDLE hProcess = OpenProcess(
        PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ,
        FALSE, processId);
    if (!hProcess) {
        return;
    }

    // Step 5: Query executable name
    char processName[MAX_PATH] = {0};
    if (GetModuleBaseNameA(hProcess, NULL, processName, sizeof(processName))) {
        QString baseName = QString::fromLocal8Bit(processName).trimmed();
        if (baseName.endsWith(".exe", Qt::CaseInsensitive))
            baseName.chop(4);
        qDebug() << "[AppTracker] Base name without extension:" << baseName;

        if (!baseName.isEmpty()) {
            qDebug() << "[AppTracker] Emitting appChanged for:" << baseName;
            emit instance->appChanged(baseName);
        }
    } else {
        qDebug() << "[AppTracker] GetModuleBaseNameA failed. Error:" << GetLastError();
    }

    CloseHandle(hProcess);
}

AppTracker::AppTracker(QObject *parent) : QObject(parent) {
    qDebug() << "[AppTracker] Constructor called.";
    qDebug() << "[AppTracker] constructor this =" << this;
    instance = this;
    startTracking();
}

AppTracker::~AppTracker() {
    qDebug() << "[AppTracker] Destructor called.";
    stopTracking();
}

void AppTracker::startTracking() {
    if (winEventHook) {
        qDebug() << "[AppTracker] Already tracking.";
        return;
    }
    qDebug() << "[AppTracker] Setting WinEvent hook.";
    winEventHook = SetWinEventHook(
        EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND,
        NULL,
        handleWinEvent,
        0, 0,
        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS
        );
    if (!winEventHook)
        qDebug() << "[AppTracker] SetWinEventHook failed. Error:" << GetLastError();
    else
        qDebug() << "[AppTracker] WinEvent hook set successfully.";
}

void AppTracker::stopTracking() {
    if (winEventHook) {
        qDebug() << "[AppTracker] Unhooking WinEvent.";
        UnhookWinEvent(winEventHook);
        winEventHook = nullptr;
    } else {
        qDebug() << "[AppTracker] No hook to unhook.";
    }
}
