#import "apptracker.h"
#import <Cocoa/Cocoa.h>


AppTracker::AppTracker(QObject *parent) : QObject(parent) {
    startTracking();
}

AppTracker::~AppTracker() {
    stopTracking();
}

void AppTracker::startTracking() {

  // Create NSWorkspace instance
  NSWorkspace *workspace = [NSWorkspace sharedWorkspace];

  // Register the app to listen for NSWorkspaceDidActivateApplicationNotification events
  [[workspace notificationCenter] addObserverForName:NSWorkspaceDidActivateApplicationNotification
                                              object:nil
                                               queue:[NSOperationQueue mainQueue]
                                          usingBlock:^(NSNotification *notification) {

      NSDictionary *userInfo = [notification userInfo];
      // Extracts the application name from userInfo
      NSRunningApplication *activeApp = [userInfo objectForKey:NSWorkspaceApplicationKey];

      if (activeApp) {
        QString appName = QString::fromNSString(activeApp.localizedName);
        // Emits a signal "appChanged" to notify QT about the change
        emit appChanged(appName);
      }

    }
  ];
}

void AppTracker::stopTracking() {
    NSWorkspace *workspace = [NSWorkspace sharedWorkspace];
    [[workspace notificationCenter] removeObserver:(id)this];
}
