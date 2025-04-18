#ifndef APPTRACKER_H
#define APPTRACKER_H

#include <QObject>
#include <QString>

class AppTracker : public QObject {
    Q_OBJECT

public:
    AppTracker(QObject *parent = nullptr);
    ~AppTracker();
    Q_INVOKABLE QString getAppIconPath(const QString& appPath);

signals:
    void appChanged(const QString &appName);  // Signal emitted when the active app changes

private:
    void startTracking();  // Platform-specific implementation for tracking apps
    void stopTracking();

    QString lastAppName;
};

#endif // APPTRACKER_H
