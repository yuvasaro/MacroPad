#include "macro.h"
#include <iostream>
#include <cstdlib>

using namespace std;

Macro::Macro() : type("defaultType"), content("defaultContent"), callback(nullptr) {}


Macro::Macro(const string& userType, const string& userContent) : type(userType), content(userContent) {}

Macro::~Macro() {}

void Macro::setType(const string& newType)  {
    type = newType;
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




