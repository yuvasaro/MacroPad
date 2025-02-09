#ifndef MACRO_H
#define MACRO_H

#include <string>
#include <functional>

class Macro {
private:
    std::string type;
    std::string content;
    std::function<void()> callback;

public:
    Macro(const std::string& userType, const std::string& userContent);
    ~Macro();

    void keystrokeCallback();
    void programCallback();

    std::string getType() const;
    std::string getContent() const;
};

#endif // MACRO_H
