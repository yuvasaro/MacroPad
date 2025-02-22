#include "serialhandler.h"
#include <QSerialPortInfo>
#include <QDebug>

SerialHandler::SerialHandler(QObject *parent)
    : QObject(parent),
    serial(new QSerialPort(this))
{
    // Example configuration; adjust port name & baud rate as needed
    serial->setPortName("COM3");          // or "/dev/ttyACM0" etc.
    serial->setBaudRate(QSerialPort::Baud9600);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    // Try to open the port
    if (serial->open(QIODevice::ReadOnly)) {
        qDebug() << "Serial port opened successfully on" << serial->portName();
        connect(serial, &QSerialPort::readyRead, this, &SerialHandler::readSerialData);
    } else {
        qDebug() << "Error opening serial port:" << serial->errorString();
    }
}

SerialHandler::~SerialHandler()
{
    // Clean up
    if (serial->isOpen()) {
        serial->close();
    }
}

void SerialHandler::readSerialData()
{
    // Read any available data
    QByteArray data = serial->readAll();
    qDebug() << "Received raw data:" << data;

    // Append to buffer for line-based parsing
    buffer.append(data);

    // Check for newline-terminated messages
    int index;
    while ((index = buffer.indexOf('\n')) != -1) {
        QByteArray line = buffer.left(index).trimmed();
        buffer.remove(0, index + 1);

        if (!line.isEmpty()) {
            bool ok;
            int buttonNum = line.toInt(&ok);
            if (ok) {
                qDebug() << "Parsed button number:" << buttonNum;
                emit buttonPressed(buttonNum);
            } else {
                qDebug() << "Failed to parse integer from line:" << line;
            }
        }
    }
}
