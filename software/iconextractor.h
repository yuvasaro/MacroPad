#ifndef ICONEXTRACTOR_H
#define ICONEXTRACTOR_H

#include <QObject>
#include <QString>

class IconExtractor : public QObject {
    Q_OBJECT
public:
    explicit IconExtractor(QObject* parent = nullptr);  // Add constructor
    Q_INVOKABLE QString extractIconForApp(const QString& appPath);  // QML-accessible
};

#endif // ICONEXTRACTOR_H
