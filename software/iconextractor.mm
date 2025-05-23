#import "iconextractor.h"
#import <Cocoa/Cocoa.h>
#import <QtCore/QStandardPaths>
#import <QtCore/QDir>
#import <QtCore/QFile>
#import <QtCore/QFileInfo>

IconExtractor::IconExtractor(QObject* parent) : QObject(parent) {}

QString IconExtractor::extractIconForApp(const QString& appPath) {
    @autoreleasepool {
        NSString *path = [NSString stringWithUTF8String:appPath.toUtf8().constData()];
        NSImage *appIcon = [[NSWorkspace sharedWorkspace] iconForFile:path];
        if (!appIcon) return "";

        NSSize newSize = NSMakeSize(64, 64);
        [appIcon setSize:newSize];

        NSBitmapImageRep *imgRep = [[NSBitmapImageRep alloc] initWithData:[appIcon TIFFRepresentation]];
        NSData *pngData = [imgRep representationUsingType:NSBitmapImageFileTypePNG properties:@{}];

        QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
        QDir().mkpath(cacheDir);
        QString filePath = cacheDir + "/app_icon_" + QFileInfo(appPath).baseName() + ".png";

        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(reinterpret_cast<const char*>([pngData bytes]), [pngData length]);
            file.close();

            qDebug() << "Icon saved to:" << filePath;  // <- Optional debug
            return filePath;
        }

        return "";
    }
}

