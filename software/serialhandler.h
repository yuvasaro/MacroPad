#ifndef SERIALHANDLER_H
#define SERIALHANDLER_H

#include <QObject>
#include <QSerialPort>

class SerialHandler : public QObject
{
    Q_OBJECT
public:
    explicit SerialHandler(QObject *parent = nullptr);
    ~SerialHandler();

signals:
    void buttonPressed(int button);

private slots:
    void readSerialData();

private:
    QSerialPort *serial;
    QByteArray buffer;
};

#endif // SERIALHANDLER_H
