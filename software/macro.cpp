#include "macro.h"
#include <iostream>
#include <cstdlib>

using namespace std;

Macro::Macro() : type("defaultType"), content("defaultContent"), callback(nullptr) {}


Macro::Macro(const string& userType, const string& userContent) : type(userType), content(userContent) {
cout << "In Macro constructor: type='" << type << "', content='" << content << "'" << endl;
   cout << "in constructor: Memory address of this Macro object: " << this << endl;
    if (type == "keystroke") {
        callback = bind(&Macro::keystrokeCallback, this);
    } else if (type == "program") {
        callback = bind(&Macro::programCallback, this);
    } else {
        callback = nullptr;
    }

}

Macro::~Macro() {

}

void Macro::keystrokeCallback() {

}

void Macro::programCallback() {
    cout << "in Macro::programCallback: type='" << type << "', content='" << content << "'" << endl;
    cout << "Memory address of this Macro object: " << this << endl;
    if (content.empty()) {
        cout << "Content length: " << content.length() << endl;
        cerr << "Content is empty!" << endl;
    }

    if (system(nullptr)) {
        system(content.c_str());
    } else {
        cerr << "system() is not available on this system!";
    }
}

void Macro::setType(const string& newType)  {
    cout << "in Macro::newType type='" << type << "', content='" << content << "'" << endl;
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
    cout << "In Macro::newType type='" << type << "', content='" << content << "'" << endl;
}

string Macro::getType() {
    return type;
}

string Macro::getContent() {
    return content;
}

void Macro::runCallback() {
    cout << "in Macro::runCallback: type='" << type << "', content='" << content << "'" << endl;
    if (callback) {
        this->callback();
    } else {
        cout << "No callback set!" << endl;
    }
}



