#include "macro.h"
#include <iostream>
#include <cstdlib>
#ifdef _WIN32
#include <minwindef.h>
#include <windows.h>
#endif

using namespace std;

Macro::Macro() : type("defaultType"), content("defaultContent"), callback(nullptr) {}


Macro::Macro(const string& userType, const string& userContent) : type(userType), content(userContent) {

    if (type == "keystroke") {
        callback = bind(&Macro::keystrokeCallback, this);
    } else if (type == "program") {
        callback = bind(&Macro::programCallback, this);
    } else {
        callback = nullptr;
    }

}

Macro::~Macro() {}

void Macro::keystrokeCallback() {

}

void Macro::programCallback() {

    if (content.empty()) {
        cerr << "Content is empty!" << endl;
    }

    if (system(nullptr)) {
        system(content.c_str());
    } else {
        cerr << "system() is not available on this system!";
    }
}

void Macro::setType(const string& newType)  {
    type = newType;

    if (type == "keystroke") {
        callback = bind(&Macro::keystrokeCallback, ref(*this));
    } else if (type == "program") {
        callback = bind(&Macro::programCallback, ref(*this));
    } else {
        callback = nullptr;
    }
}

void Macro::setContent(const string& newContent) {
    content = newContent;
}

string Macro::getType() {
    return type;
}

string Macro::getContent() {
    return content;
}

 vector<WORD> Macro::translateKeys(const string& content) {
    vector<string> strs;
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

    vector<WORD> keys;
    vector<pair<string, WORD>> keyMap = {
            {"CTRL", VK_CONTROL},
            {"ALT", VK_MENU},
            {"SHIFT", VK_SHIFT},
            {"ENTER", VK_RETURN},
            {"TAB", VK_TAB},
            {"ESC", VK_ESCAPE},
            {"SPACE", VK_SPACE},
            {"BACKSPACE", VK_BACK}
        };

    for(const string& s : strs){
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

    for (WORD key : keys) {
        cout << key << " ";
    }

    return keys;
}

vector<WORD> result = Macro::translateKeys("H+CTRL+C");




