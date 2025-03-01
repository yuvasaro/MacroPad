#ifndef MACRO_H
#define MACRO_H

#ifdef _WIN32
#include <minwindef.h>
#include <windows.h>
#endif
#include <string>
#include <functional>
#include <QObject>
#include <QString>

class Macro : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString type READ getType WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(QString content READ getContent WRITE setContent NOTIFY contentChanged)

public:
    explicit Macro(QObject* parent = nullptr);
    Macro(const QString& userType, const QString& userContent, QObject* parent = nullptr);
    ~Macro();

    QString getType() const;
    void setType(const QString& newType);

    QString getContent() const;
    void setContent(const QString& newContent);
    static std::vector<WORD> translateKeys(const std::string& content);


signals:
    void typeChanged();
    void contentChanged();

    void runCallback();


private:
    QString type;
    QString content;

};

#endif
