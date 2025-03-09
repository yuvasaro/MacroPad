#include "appTracker.h"
#include <QTimer>
#include <windows.h>

AppTracker::AppTracker(QObject *parent) : QObject(parent), lastAppName("") {
    startTracking();
}

AppTracker::~AppTracker() {
    stopTracking();
}

void AppTracker::startTracking() {
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        HWND hwnd = GetForegroundWindow();
        if (hwnd) {
            char windowTitle[256];
            GetWindowTextA(hwnd, windowTitle, sizeof(windowTitle));

            QString appName = QString::fromLocal8Bit(windowTitle).trimmed(); // Remove leading/trailing spaces

            // Emit only when appName actually changes
            if (!appName.isEmpty() && appName != lastAppName) {
                lastAppName = appName;
                emit appChanged(appName);
            }
        }
    });
    timer->start(500); // Poll every 500ms
}

void AppTracker::stopTracking() {
    // No specific cleanup needed since QTimer is managed by Qt's parent system.
}
