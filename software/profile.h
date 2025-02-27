#ifndef PROFILE_H
#define PROFILE_H

#include "macro.h"
#include <memory>
#include <map>
#include <QString>

class Profile {
private:

    QString name;
    std::map<int, std::unique_ptr<Macro>> macros;

public:

    Profile(const Profile&) = delete;
    Profile& operator=(const Profile&) = delete;

    Profile() = default;
    Profile(Profile&&) = default;
    Profile& operator=(Profile&&) = default;

    Profile(const QString& userName);
    ~Profile() = default;

    void setMacro(int keyNum, const QString& type, const QString& content);
    void deleteMacro(int keyNum);
    std::unique_ptr<Macro>& getMacro(int keyNum);

    void saveProfile();
    static Profile loadProfile(const QString& filePath);
};

#endif // PROFILE_H
