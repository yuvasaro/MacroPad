#ifndef IMAGECACHE_H
#define IMAGECACHE_H

#include <QObject>
#include <QString>
#include <QMap>

class imagecache : public QObject {
    Q_OBJECT
public:
    static imagecache* instance();
    QString cacheImage(const QString& originalPath);

private:
    explicit imagecache(QObject* parent = nullptr);
    static imagecache* m_instance;
    QMap<QString, QString> m_cache;
};

#endif // IMAGECACHE_H
