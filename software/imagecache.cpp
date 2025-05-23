#include "imagecache.h"
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>

ImageCache* ImageCache::m_instance = nullptr;

ImageCache::ImageCache(QObject* parent) : QObject(parent) {}

ImageCache* ImageCache::instance() {
    if (!m_instance) {
        m_instance = new ImageCache();
    }
    return m_instance;
}

QString ImageCache::cacheImage(const QString& originalPath) {
    if (m_cache.contains(originalPath)) {
        return m_cache[originalPath];
    }

    QFileInfo fileInfo(originalPath);
    if (!fileInfo.exists()) {
        return "";
    }

    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir dir(cacheDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QString cachedPath = cacheDir + "/" + fileInfo.fileName();
    if (QFile::copy(originalPath, cachedPath)) {
        m_cache[originalPath] = cachedPath;
        return cachedPath;
    }

    return "";
}
