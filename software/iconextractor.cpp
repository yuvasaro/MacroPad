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
#include <shellapi.h>
#include <QUrl>

QPixmap HICONToQPixmap(HICON hIcon) {
    if (!hIcon) {
        qDebug() << "[IconExtractor]: Null HICON received";
        return QPixmap();
    }

    // Get icon info
    ICONINFO iconInfo;
    if (!GetIconInfo(hIcon, &iconInfo)) {
        qDebug() << "[IconExtractor]: GetIconInfo failed";
        return QPixmap();
    }

    // Get bitmap dimensions
    BITMAP bmp;
    if (!GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bmp)) {
        qDebug() << "[IconExtractor]: GetObject failed";
        DeleteObject(iconInfo.hbmColor);
        DeleteObject(iconInfo.hbmMask);
        return QPixmap();
    }

    int width = bmp.bmWidth;
    int height = bmp.bmHeight;

    // Create a QImage with the correct dimensions
    QImage image(width, height, QImage::Format_ARGB32);

    // Create a device context for the image
    HDC hdc = GetDC(NULL);
    HDC hMemDC = CreateCompatibleDC(hdc);

    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // Negative for top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pBits = nullptr;
    HBITMAP hBitmap = CreateDIBSection(hMemDC, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);

    if (!hBitmap || !pBits) {
        qDebug() << "[IconExtractor]: Failed to create DIB section";
        DeleteDC(hMemDC);
        ReleaseDC(NULL, hdc);
        DeleteObject(iconInfo.hbmColor);
        DeleteObject(iconInfo.hbmMask);
        return QPixmap();
    }

    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

    // Draw the icon
    DrawIconEx(hMemDC, 0, 0, hIcon, width, height, 0, NULL, DI_NORMAL);

    // Copy pixel data and swap R/B
    uchar* srcData = (uchar*)pBits;
    for (int y = 0; y < height; ++y) {
        QRgb* destLine = (QRgb*)image.scanLine(y);
        uchar* srcLine = srcData + (y * width * 4);

        for (int x = 0; x < width; ++x) {
            uchar b = srcLine[x * 4 + 0];
            uchar g = srcLine[x * 4 + 1];
            uchar r = srcLine[x * 4 + 2];
            uchar a = srcLine[x * 4 + 3];

            destLine[x] = qRgba(r, g, b, a);
        }
    }

    QPixmap pixmap = QPixmap::fromImage(image);

    // Cleanup
    SelectObject(hMemDC, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hMemDC);
    ReleaseDC(NULL, hdc);
    DeleteObject(iconInfo.hbmColor);
    DeleteObject(iconInfo.hbmMask);

    return pixmap;
}


IconExtractor::IconExtractor(QObject* parent) : QObject(parent) {}

QString IconExtractor::extractIconForApp(const QString& appPath) {
    // Remove leading slash if present (from file:/// URLs converted by QML)
    QString cleanPath = appPath;
    if (cleanPath.startsWith("/") && cleanPath.length() > 2 && cleanPath[2] == ':') {
        cleanPath = cleanPath.mid(1);
    }

    // Convert forward slashes to backslashes for Windows
    cleanPath = QDir::toNativeSeparators(cleanPath);

    // Check if file exists
    if (!QFile::exists(cleanPath)) {
        qDebug() << "[IconExtractor]: File does not exist:" << cleanPath;
        return "";
    }

    HICON hIconLarge = nullptr;

    // Try method 1: ExtractIconExW
    UINT iconCount = ExtractIconExW((LPCWSTR)cleanPath.utf16(), 0, &hIconLarge, nullptr, 1);

    // If ExtractIconExW failed (returns -1 as 0xFFFFFFFF), try SHGetFileInfo
    if (iconCount == 0xFFFFFFFF || !hIconLarge) {
        SHFILEINFOW sfi = {0};
        DWORD_PTR result = SHGetFileInfoW(
            (LPCWSTR)cleanPath.utf16(),
            0,
            &sfi,
            sizeof(sfi),
            SHGFI_ICON | SHGFI_LARGEICON
            );

        if (result && sfi.hIcon) {
            hIconLarge = sfi.hIcon;
        } else {
            qDebug() << "[IconExtractor]: Failed to extract icon from:" << cleanPath;
            return "";
        }
    }

    if (!hIconLarge) {
        qDebug() << "[IconExtractor]: No icon handle obtained";
        return "";
    }

    QPixmap pixmap = HICONToQPixmap(hIconLarge);
    DestroyIcon(hIconLarge);

    if (pixmap.isNull()) {
        qDebug() << "[IconExtractor]: Failed to convert icon to pixmap";
        return "";
    }

    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (!QDir().mkpath(cacheDir)) {
        qDebug() << "[IconExtractor]: Failed to create cache directory";
        return "";
    }

    QString fileName = "app_icon_" + QFileInfo(cleanPath).baseName() + ".png";
    QString filePath = cacheDir + "/" + fileName;

    if (pixmap.save(filePath, "PNG")) {
        if (QFile::exists(filePath)) {
            qDebug() << "[IconExtractor]: Icon saved successfully for" << QFileInfo(cleanPath).fileName();
            return QUrl::fromLocalFile(filePath).toString();
        }
    }

    qDebug() << "[IconExtractor]: Failed to save icon";
    return "";
}
#endif // Q_OS_WIN
