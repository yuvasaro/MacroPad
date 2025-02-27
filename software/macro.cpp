#include "macro.h"
#include <iostream>
#include <cstdlib>


Macro::Macro() : type("defaultType"), content("defaultContent") {}

Macro::Macro(const QString& userType, const QString& userContent) : type(userType), content(userContent) {}

Macro::~Macro() {}

void Macro::setType(const QString& newType)  {
    type = newType;
}

void Macro::setContent(const QString& newContent) {
    content = newContent;
}

QString Macro::getType() {
    return type;
}

QString Macro::getContent() {
    return content;
}




