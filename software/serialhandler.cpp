// SerialHandler.cpp  — minimal patch to support ESP32 + UNO safely
#include "serialhandler.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
#include <QFile>


static constexpr qint32 BAUD_UNO   = QSerialPort::Baud19200;
static constexpr qint32 BAUD_ESP32 = QSerialPort::Baud115200;

SerialHandler::SerialHandler(QObject *parent)
    : QObject(parent), COMPORT(nullptr)
{
    connect(&scanTimer, &QTimer::timeout, this, &SerialHandler::tryConnect);
    connect(&heartbeatTimer, &QTimer::timeout, this, &SerialHandler::heartbeat);
    startScanning();
}

SerialHandler::~SerialHandler()
{
    stopScanning();
    closePort();
}

void SerialHandler::startScanning()
{
    if(!scanTimer.isActive())
    {
        scanTimer.start(2000);
        qDebug() << "[Serial] scanning for devices ...";
    }
}

void SerialHandler::stopScanning()
{
    scanTimer.stop();
}

bool SerialHandler::isCandidate(const QSerialPortInfo &info) const
{
    const QString d = info.description();
    const bool isUNO = d.startsWith("UNO", Qt::CaseInsensitive);
    const bool isCP210 = d.contains("CP210", Qt::CaseInsensitive);
    const bool isCH34 = d.contains("CH34", Qt::CaseInsensitive);
    return isUNO || isCP210 || isCH34;
}

bool SerialHandler::openPort(const QSerialPortInfo &info)
{
    closePort();
    COMPORT = new QSerialPort(info, this);

    const bool useUNO = info.description().startsWith("UNO", Qt::CaseInsensitive);
    const qint32 baud = useUNO? BAUD_UNO : BAUD_ESP32;

    COMPORT->setBaudRate(baud);
    COMPORT->setParity(QSerialPort::NoParity);
    COMPORT->setDataBits(QSerialPort::Data8);
    COMPORT->setStopBits(QSerialPort::OneStop);
    COMPORT->setFlowControl(QSerialPort::NoFlowControl);

    if(!COMPORT->open(QIODevice::ReadWrite))
    {
        qDebug() << "[Serial] open failed on" << info.portName() << ":" << COMPORT->errorString();
        COMPORT->deleteLater();
        COMPORT = nullptr;
        return false;
    }

    connect(COMPORT, &QSerialPort::readyRead, this, &SerialHandler::readSerialData);
    connect(COMPORT, &QSerialPort::errorOccurred, this, [this](QSerialPort::SerialPortError e){
        if (e == QSerialPort::ResourceError || e == QSerialPort::DeviceNotFoundError) {
            qDebug() << "[Serial] device lost; resuming scan …";
            closePort();
            startScanning();
            emit connectedChanged(false);
        }
    });

    qDebug()<<"[Serial] connected to" << info.portName() << "@" << baud << "(" << info.description() << ")";
    heartbeatTimer.start(30000);
    stopScanning();
    emit connectedChanged(true);
    return true;
}

void SerialHandler::closePort()
{
    heartbeatTimer.stop();
    if (COMPORT) {
        if (COMPORT->isOpen()) COMPORT->close();
        COMPORT->deleteLater();
        COMPORT = nullptr;
    }
}

void SerialHandler::tryConnect()
{
    for (const auto &info : QSerialPortInfo::availablePorts()) {
        if (!isCandidate(info)) continue;
        if (openPort(info)) return; // success
    }
}

void SerialHandler::heartbeat()
{
    qDebug() << "[Serial] heartbeat:" << (COMPORT && COMPORT->isOpen() ? "connected" : "disconnected");
}


// ---------------- your original methods remain unchanged ----------------
void SerialHandler::readSerialData()
{
    if(!COMPORT) return;
    QByteArray data = COMPORT->readAll();
    qDebug() << "Received data (raw bytes):" << data;

    QString trimmedText = QString::fromUtf8(data).trimmed();
    QByteArray butt = trimmedText.toUtf8();
    qDebug() << "Number only:" << butt;
    bool ok = false;
    int number = trimmedText.toInt(&ok);
    if(number == 1) this->transferAllImages("images", 6, 6);
    qDebug() << "Integer:" << number;

    if(ok) emit dataReceived(number);
}

void SerialHandler::sendProfile(int profileCode)
{
    if (COMPORT && COMPORT->isOpen()) {
        QByteArray data = QByteArray::number(profileCode);
        qDebug() << "Sending profile code:" << profileCode;
        COMPORT->write(data);
    } else {
        qDebug() << "Serial port not open";
    }
}

/* ---------- binary write utility ---------- */
bool SerialHandler::writeAll(QSerialPort *port, const QByteArray &data, int chunk, int timeoutMs)
{
    int sent = 0;
    while (sent < data.size()) {
        const int n = qMin(chunk, data.size() - sent);
        const qint64 w = port->write(data.constData() + sent, n);
        if (w < 0) return false;
        if (!port->waitForBytesWritten(timeoutMs)) return false;
        sent += int(w);
    }
    return true;
}

bool SerialHandler::enterImageMode()    // 47
{
    if (!COMPORT || !COMPORT->isOpen()) return false;
    const char c = 47; // single byte
    return writeAll(COMPORT, QByteArray(1, c));
}

bool SerialHandler::sendImageSlot(quint8 profileX, quint8 iconY, const QByteArray &pngBytes)
{
    if (!COMPORT || !COMPORT->isOpen()) return false;
    // header: x y (raw bytes)
    QByteArray hdr;
    hdr.reserve(2);
    hdr.append(char(profileX));
    hdr.append(char(iconY));
    if (!writeAll(COMPORT, hdr)) return false;

    // body: raw PNG
    if (!writeAll(COMPORT, pngBytes)) return false;

    // terminator: 48
    const char endImage = 48;
    return writeAll(COMPORT, QByteArray(1, endImage));
}

bool SerialHandler::exitImageModeAndReset()   // 49
{
    if (!COMPORT || !COMPORT->isOpen()) return false;
    const char c = 49;
    return writeAll(COMPORT, QByteArray(1, c));
}

QString SerialHandler::pngPathFor(int x, int y, const QString &root, bool oneBasedY)
{
    // your layout: images/profile0/1.png..6.png (y is 1-based in filenames)
    const int fnameIndex = oneBasedY ? (y + 1) : y;
    return QString("%1/profile%2/%3.png").arg(root).arg(x).arg(fnameIndex);
}

bool SerialHandler::isPng(const QByteArray &bytes)
{
    static const QByteArray sig = QByteArray::fromHex("89504E470D0A1A0A");
    return bytes.size() >= 8 && bytes.left(8) == sig;
}

/* ---------- bulk transfer ---------- */
bool SerialHandler::transferAllImages(const QString &root, int profiles, int iconsPerProfile)
{
    if (!COMPORT || !COMPORT->isOpen()) { qDebug() << "[Images] no serial connection"; return false; }

    if (!enterImageMode()) { qDebug() << "[Images] failed 47"; return false; }

    bool ok = true;
    for (int x = 0; x < profiles && ok; ++x) {
        for (int y = 0; y < iconsPerProfile && ok; ++y) {
            const QString path = pngPathFor(x, y, root, /*oneBasedY*/true);
            QFile f(path);
            if (!f.exists()) { qDebug() << "[Images] missing" << path << "(skipping)"; continue; }
            if (!f.open(QIODevice::ReadOnly)) { qDebug() << "[Images] open failed" << path; ok = false; break; }
            const QByteArray bytes = f.readAll();
            f.close();

            if (!isPng(bytes)) { qDebug() << "[Images] not a PNG:" << path; ok = false; break; }

            qDebug() << "[Images]" << path << "-> (" << x << "," << y << ") bytes:" << bytes.size();
            if (!sendImageSlot(quint8(x), quint8(y), bytes)) { qDebug() << "[Images] send failed"; ok = false; break; }
        }
    }

    if (ok) {
        if (!exitImageModeAndReset()) { qDebug() << "[Images] failed 49"; ok = false; }
        else qDebug() << "[Images] transfer complete; reset requested.";
    }
    return ok;
}
