#include "imagecache.h"
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>

imagecache* imagecache::m_instance = nullptr;

imagecache::imagecache(QObject* parent) : QObject(parent) {}

imagecache* imagecache::instance() {
    if (!m_instance) {
        m_instance = new imagecache();
    }
    return m_instance;
}

QString imagecache::cacheImage(const QString& originalPath) {
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
