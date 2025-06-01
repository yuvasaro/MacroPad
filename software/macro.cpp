#include "macro.h"
#include <cstdlib>
#ifdef _WIN32
#include <minwindef.h>
#include <windows.h>
#endif


Macro::Macro(QObject* parent) : QObject(parent), type("defaultType"), content("defaultContent") {}

Macro::Macro(const QString& userType, const QString& userContent, const QString& userImagePath, QObject* parent)
    : QObject(parent), type(userType), content(userContent), imagePath(userImagePath) {}

Macro::~Macro() {}

void Macro::setType(const QString& newType) {
    if (type != newType) {
        type = newType;
        emit typeChanged();
    }
}

void Macro::setContent(const QString& newContent) {
    if (content != newContent) {
        content = newContent;
        emit contentChanged();
    }
}

void Macro::setImagePath(const QString& path) {
    if (imagePath != path) {
        imagePath = path;
        emit imagePathChanged();
    }
}

QString Macro::getType() const {
    return type;
}

QString Macro::getContent() const {
    return content;
}

QString Macro::getImagePath() const {
    return imagePath;
}

QString Macro::toString() const {
    return "Type: " + QString(type) + " Content: " + QString(content);
}


/*
 std::vector<WORD> Macro::translateKeys(const std::string& content) {
    std::vector<std::string> strs;
    int start = 0;

    for (int end = 0; end < content.size(); ++end) {
        if (content[end] == '+') {
            if (start < end) {
                strs.push_back(content.substr(start, end - start));
            }
            start = end + 1;
        }
    }

    if (start < content.size()) {
        strs.push_back(content.substr(start));
    }

    std::vector<WORD> keys;
    std::vector<std::pair<std::string, WORD>> keyMap = {
            {"CTRL", VK_CONTROL},
            {"ALT", VK_MENU},
            {"SHIFT", VK_SHIFT},
            {"ENTER", VK_RETURN},
            {"TAB", VK_TAB},
            {"ESC", VK_ESCAPE},
            {"SPACE", VK_SPACE},
            {"BACKSPACE", VK_BACK}
        };

    for(const std::string& s : strs){
        if(s.size()==1){
            keys.push_back(static_cast<WORD>(s[0]));
        } else {
            for(const auto& pair : keyMap){
                if (pair.first==s){
                    keys.push_back(pair.second);
                }
            }
        }
    }


    return keys;
}

*/
