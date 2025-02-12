#ifndef PROFILE_H
#define PROFILE_H

#include "macro.h"
#include <string>
#include <unordered_map>

using namespace std;
class Profile {
private:

    string name;
    unordered_map<int, Macro> macros;

public:
    Profile(const string& userName);
    ~Profile() = default;

    void setMacro(int keyNum, const string& type, const string& content);
    void deleteMacro(int keyNum);
    void runMacro(int keyNum);

    void saveProfile(const string& filePath);
    static Profile loadProfile(const string& filePath);
};

#endif // PROFILE_H
