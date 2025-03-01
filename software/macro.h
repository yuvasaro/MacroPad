#ifndef MACRO_H
#define MACRO_H

#ifdef _WIN32
#include <minwindef.h>
#include <windows.h>
#endif
#include <string>
#include <functional>


using namespace std;

class Macro {
private:
    string type;
    string content;

public:

    function<void()> callback;
    Macro();
    Macro(const string& userType, const string& userContent);
    ~Macro();

    void keystrokeCallback();
    void programCallback();

    void setType(const string& newType);
    void setContent(const string& newContent);

    string getType();
    string getContent();

    void runCallback();

    static vector<WORD> translateKeys(const string& content);
};

#endif // MACRO_H
