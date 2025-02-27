#ifndef MACRO_H
#define MACRO_H

#include <string>
#include <functional>


using namespace std;

class Macro {
private:
    string type;
    string content;

public:
    Macro();
    Macro(const string& userType, const string& userContent);
    ~Macro();

    void setType(const string& newType);
    void setContent(const string& newContent);

    string getType();
    string getContent();

    void runCallback();
};

#endif // MACRO_H
