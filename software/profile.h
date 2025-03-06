#ifndef PROFILE_H
#define PROFILE_H

#include "macro.h"
<<<<<<< Updated upstream
#include <string>
#include <map>
=======
#include <memory>
#include <QMap>
#include <QString>
#include <QObject>

class Profile : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name READ getName WRITE setName NOTIFY nameChanged)
>>>>>>> Stashed changes

using namespace std;
class Profile {
private:

<<<<<<< Updated upstream
    string name;
    map<int, unique_ptr<Macro>> macros;
=======
    QString name;
    QMap<int, QSharedPointer<Macro>> macros;
>>>>>>> Stashed changes

public:

    // a bunch of chat gpt code that helped me debug the pointer issue somehow???
    Profile(const Profile&) = delete;
    Profile& operator=(const Profile&) = delete;
<<<<<<< Updated upstream

    Profile() = default;
    Profile(Profile&&) = default;
    Profile& operator=(Profile&&) = default;
    // ----------------------------------------

    Profile(const string& userName);
    ~Profile() = default;
=======
    Profile(Profile&&) = delete;
    Profile& operator=(Profile&&) = delete;


    Profile() = default;

    Q_INVOKABLE QString getName() const;
    Q_INVOKABLE void setName(const QString& newName);
    Q_INVOKABLE void setMacro(int keyNum, const QString& type, const QString& content);
    Q_INVOKABLE void deleteMacro(int keyNum);
    Q_INVOKABLE QSharedPointer<Macro> getMacro(int keyNum);
    Q_INVOKABLE void saveProfile();
    Q_INVOKABLE static Profile* loadProfile(const QString& nameLookUp);
>>>>>>> Stashed changes

    void setMacro(int keyNum, const string& type, const string& content);
    void deleteMacro(int keyNum);
    void runMacro(int keyNum);

    void saveProfile(const string& filePath);
    static Profile loadProfile(const string& filePath);

    Macro getMacro(int keyNum);
};

#endif // PROFILE_H
<<<<<<< Updated upstream
=======




>>>>>>> Stashed changes
