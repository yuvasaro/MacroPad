#ifndef MACRO_H
#define MACRO_H

#include <QString>

class Macro {
private:
    QString type;
    QString content;

public:
    Macro();
    Macro(const QString& userType, const QString& userContent);
    ~Macro();

    void setType(const QString& newType);
    void setContent(const QString& newContent);

    QString getType();
    QString getContent();

    void runCallback();
};

#endif // MACRO_H
