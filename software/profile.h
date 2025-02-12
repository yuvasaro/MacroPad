#ifndef PROFILE_H
#define PROFILE_H

#include "Macro.h"
#include <string>
#include <unordered_map>

using namespace std;
class Profile {
private:

    string name;
    unordered_map<int, unique_ptr<Macro>> macros;

public:
    Profile(const string& userName);
    ~Profile();

    void setMacro(int keyNum, string& type, string& content);
    void deleteMacro(int keyNum);
    void runMacro(int keyNum);

    void saveProfile(string& filePath);
    static Profile loadProfile(string& );
};

#endif // PROFILE_H
