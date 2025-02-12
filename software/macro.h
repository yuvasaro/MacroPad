#ifndef MACRO_H
#define MACRO_H

#include <string>
#include <functional>

using namespace std;

class Macro {
private:
    string type;
    string content;
    function<void()> callback;

public:
    Macro(const string& userType, const string& userContent);
    ~Macro();

    void keystrokeCallback();
    void programCallback();

    void setType(const string& newType);
    void setContent(const string& newContent);
};

#endif // MACRO_H
