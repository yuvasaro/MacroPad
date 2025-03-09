#ifndef PROFILE_H
#define PROFILE_H

#include "macro.h"
#include <memory>
#include <QMap>
#include <QString>
#include <QObject>

class Profile : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name READ getName WRITE setName NOTIFY nameChanged)

private:

    QString name;
<<<<<<< HEAD
    QString app;
    std::map<int, std::unique_ptr<Macro>> macros;
=======
    QMap<int, QSharedPointer<Macro>> macros;
>>>>>>> f4c04bcf4479a8ab51b4e812c7eb614a866e0aae

public:
    explicit Profile(QObject* parent = nullptr);
    Profile(const QString& userName, const QString& appName, QObject* parent = nullptr);
    ~Profile() = default;

    Profile(const Profile&) = delete;
    Profile& operator=(const Profile&) = delete;
    Profile(Profile&&) = delete;
    Profile& operator=(Profile&&) = delete;


    Profile() = default;

    Q_INVOKABLE QString getName() const;
    Q_INVOKABLE QString getApp() const;
    Q_INVOKABLE void setName(const QString& newName);
    Q_INVOKABLE void setApp(const QString& newApp);
    Q_INVOKABLE void setMacro(int keyNum, const QString& type, const QString& content);
    Q_INVOKABLE void deleteMacro(int keyNum);
    Q_INVOKABLE QSharedPointer<Macro> getMacro(int keyNum);
    Q_INVOKABLE void saveProfile();
    Q_INVOKABLE static Profile* loadProfile(const QString& nameLookUp);

signals:
    void nameChanged();
    void appChanged();

};

#endif // PROFILE_H

