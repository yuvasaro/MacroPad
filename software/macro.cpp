#include "macro.h"
#include <iostream>
#include <cstdlib>

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




