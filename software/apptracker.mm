#import "apptracker.h"
#import <Cocoa/Cocoa.h>


  AppTracker::AppTracker(QObject *parent) : QObject(parent) {
      startTracking();
  }

  AppTracker::~AppTracker() {
      stopTracking();
  }

  void AppTracker::startTracking() {

    //create NSWorkspace instance
    NSWorkspace *workspace = [NSWorkspace sharedWorkspace];

    // register the app to listen for NSWorkspaceDidActivateApplicationNotification events
    [[workspace notificationCenter] addObserverForName:NSWorkspaceDidActivateApplicationNotification
                                                object:nil
                                                 queue:[NSOperationQueue mainQueue]
                                            usingBlock:^(NSNotification *notification) {

        NSDictionary *userInfo = [notification userInfo];
        //extracts the application name from userInfo
        NSRunningApplication *activeApp = [userInfo objectForKey:NSWorkspaceApplicationKey];

        if (activeApp) {
          QString appName = QString::fromNSString(activeApp.localizedName);
          // emits a signal "appChanged" to notify QT about the change
          emit appChanged(appName);
        }

      }
    ];
  }

void AppTracker::stopTracking() {
    NSWorkspace *workspace = [NSWorkspace sharedWorkspace];
    [[workspace notificationCenter] removeObserver:(id)this];
}

QString AppTracker::getAppIconPath(const QString& appPath) {
    NSString *nsAppPath = appPath.toNSString();

    // 1. Try to get bundle icon
    NSBundle *appBundle = [NSBundle bundleWithPath:nsAppPath];
    if (appBundle) {
        NSString *iconName = [[appBundle infoDictionary] objectForKey:@"CFBundleIconFile"];
        if (iconName) {
            if (![iconName hasSuffix:@".icns"]) {
                iconName = [iconName stringByAppendingString:@".icns"];
            }
            NSString *iconPath = [appBundle pathForResource:[iconName stringByDeletingPathExtension]
                                                    ofType:@"icns"];
            if (iconPath) {
                return QString::fromNSString(iconPath);
            }
        }
    }

    // 2. Fallback to generic executable icon
    return "";
}
