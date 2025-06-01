#ifndef IMAGECACHE_H
#define IMAGECACHE_H

#include <QObject>
#include <QString>
#include <QMap>

class ImageCache : public QObject {
    Q_OBJECT
public:
    static ImageCache* instance();
    QString cacheImage(const QString& originalPath);

private:
    explicit ImageCache(QObject* parent = nullptr);
    static ImageCache* m_instance;
    QMap<QString, QString> m_cache;
};

#endif // IMAGECACHE_H
