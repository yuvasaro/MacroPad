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

