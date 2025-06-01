#include "iconextractor.h"
#include <qpixmap.h>

#ifdef Q_OS_WIN

#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QImage>
#include <windows.h>

QPixmap HICONToQPixmap(HICON hIcon) {
    if (!hIcon) return QPixmap();

    ICONINFO iconInfo;
    if (!GetIconInfo(hIcon, &iconInfo)) return QPixmap();

    BITMAP bmp;
    GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bmp);

    int width = bmp.bmWidth;
    int height = bmp.bmHeight;

    HDC hdc = GetDC(NULL);
    HDC hMemDC = CreateCompatibleDC(hdc);
    HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDC, iconInfo.hbmColor);

    QImage image(width, height, QImage::Format_ARGB32);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            COLORREF color = GetPixel(hMemDC, x, y);
            image.setPixel(x, y, qRgba(GetRValue(color), GetGValue(color), GetBValue(color), 255));
        }
    }

    SelectObject(hMemDC, hOldBmp);
    DeleteDC(hMemDC);
    ReleaseDC(NULL, hdc);

    DeleteObject(iconInfo.hbmColor);
    DeleteObject(iconInfo.hbmMask);

    return QPixmap::fromImage(image);
}

IconExtractor::IconExtractor(QObject* parent) : QObject(parent) {}

QString IconExtractor::extractIconForApp(const QString& appPath) {
    HICON hIcon = ExtractIconW(nullptr, (LPCWSTR)appPath.utf16(), 0);
    if (!hIcon || hIcon == (HICON)1) {
        qDebug() << "Failed to extract icon from:" << appPath;
        return "";
    }

    QPixmap pixmap = HICONToQPixmap(hIcon);
    DestroyIcon(hIcon);

    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir().mkpath(cacheDir);
    QString filePath = cacheDir + "/app_icon_" + QFileInfo(appPath).baseName() + ".png";

    if (pixmap.save(filePath, "PNG")) {
        qDebug() << "Icon saved to:" << filePath;
        return filePath;
    }

    return "";
}

#endif // Q_OS_WIN
