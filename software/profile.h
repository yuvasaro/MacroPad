#ifndef PROFILE_H
#define PROFILE_H

#include "macro.h"
#include <string>
#include <map>

using namespace std;
class Profile {
private:

    string name;
    map<int, unique_ptr<Macro>> macros;

public:

    // a bunch of chat gpt code that helped me debug the pointer issue somehow???
    Profile(const Profile&) = delete;
    Profile& operator=(const Profile&) = delete;

    Profile() = default;
    Profile(Profile&&) = default;
    Profile& operator=(Profile&&) = default;
    Profile(const string& userName);
    ~Profile() = default;
    // ----------------------------------------

    void setMacro(int keyNum, const string& type, const string& content);
    void deleteMacro(int keyNum);
    void runMacro(int keyNum);

    void saveProfile(const string& filePath);
    static Profile loadProfile(const string& filePath);

    Macro getMacro(int keyNum);
};

#endif // PROFILE_H
