// SerialHandler.cpp  — minimal patch to support ESP32 + UNO safely
#include "serialhandler.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
#include <QFile>
#include <QCoreApplication>
#include <QDir>

static constexpr qint32 BAUD_UNO   = QSerialPort::Baud19200;
static constexpr qint32 BAUD_ESP32 = QSerialPort::Baud115200;

SerialHandler::SerialHandler(QObject *parent)
    : QObject(parent), COMPORT(nullptr), m_imagesDir(QCoreApplication::applicationDirPath()+"/images")
{
    QDir().mkpath(m_imagesDir);
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
// void SerialHandler::readSerialData()
// {
//     if(!COMPORT) return;
//     QByteArray data = COMPORT->readAll();
//     qDebug() << "Received data (raw bytes):" << data;

//     QString trimmedText = QString::fromUtf8(data).trimmed();
//     QByteArray butt = trimmedText.toUtf8();
//     qDebug() << "Number only:" << butt;
//     bool ok = false;
//     int number = trimmedText.toInt(&ok);
//     if(number == 1) this->transferAllImages("images", 6, 6);
//     qDebug() << "Integer:" << number;

//     if(ok) emit dataReceived(number);
// }

void SerialHandler::readSerialData()
{
    if (!COMPORT) return;

    m_rxBuf += COMPORT->readAll();

    // process complete lines
    for (;;) {
        int nl = m_rxBuf.indexOf('\n');
        if (nl < 0) break;

        QByteArray line = m_rxBuf.left(nl);
        m_rxBuf.remove(0, nl + 1);

        line = line.trimmed();
        if (line.isEmpty()) continue;

        qDebug() << "rx line:" << line;

        // Only lines that start with '#' are input events
        if (line.startsWith('#')) {
            bool ok = false;
            int code = QString::fromUtf8(line.mid(1)).toInt(&ok);
            if(code==1) transferAllImages(m_imagesDir, /*profiles*/6, /*icons*/6);
            if (ok) {
                emit dataReceived(code);
            } else {
                qDebug() << "[Serial] bad input code:" << line;
            }
        } else {
            // everything else is a device log; ignore or show
            // qDebug() << "[ESP]" << line;
        }
    }
}

void SerialHandler::sendProfile(int profileCode)
{
    if (COMPORT && COMPORT->isOpen()) {
        QByteArray data = QByteArray::number(profileCode)+"\n";
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

bool SerialHandler::enterImageMode() {
    if (!COMPORT || !COMPORT->isOpen()) return false;
    if (!writeAll(COMPORT, QByteArray("47\n"))) return false;  // send and flush
    return waitForByte('R', 3000);                             // wait READY from ESP
}

bool SerialHandler::exitImageModeAndReset() {
    if (!COMPORT || !COMPORT->isOpen()) return false;
    return writeAll(COMPORT, QByteArray("49\n"));
}

bool SerialHandler::sendImageSlot(quint8 x, quint8 y, const QByteArray &pngBytes)
{
    if (!COMPORT || !COMPORT->isOpen()) return false;

    QByteArray pkt;
    pkt.reserve(1 + 1 + 4 + pngBytes.size());
    pkt.append(char(x));
    pkt.append(char(y));

    const quint32 len = pngBytes.size();
    pkt.append(char(len & 0xFF));
    pkt.append(char((len >> 8) & 0xFF));
    pkt.append(char((len >> 16) & 0xFF));
    pkt.append(char((len >> 24) & 0xFF));

    pkt.append(pngBytes);
    return writeAll(COMPORT, pkt);
}

bool SerialHandler::waitForByte(char want, int timeoutMs) {
    QElapsedTimer t; t.start();
    while (t.elapsed() < timeoutMs) {
        if (!COMPORT || !COMPORT->isOpen()) return false;
        if (COMPORT->waitForReadyRead(50)) {
            const QByteArray b = COMPORT->readAll();
            if (b.contains(want)) return true;
        }
    }
    return false;
}

// Flat naming: images/pXiY.png  (x = 0..5, y = 0..5)
QString SerialHandler::pngPathFor(int x, int y, const QString &root)
{
    return QDir(root).filePath(QString("p%1i%2.png").arg(x).arg(y));
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
    if (!enterImageMode())                { qDebug() << "[Images] failed to enter or no READY"; return false; }

    bool ok = true;
    for (int x = 0; x < profiles && ok; ++x) {
        for (int y = 0; y < iconsPerProfile && ok; ++y) {
            const QString path = pngPathFor(x, y, root);
            QFile f(path);
            if (!f.exists()) { qDebug() << "[Images] missing" << path << "(skipping)"; continue; }
            if (!f.open(QIODevice::ReadOnly)) { qDebug() << "[Images] open failed" << path; ok = false; break; }
            const QByteArray bytes = f.readAll();
            f.close();
            if (!isPng(bytes)) { qDebug() << "[Images] not a PNG:" << path; ok = false; break; }

            // send 6-byte header + body
            QByteArray hdr;
            hdr.reserve(6);
            hdr.append(char(x));
            hdr.append(char(y));
            const quint32 len = bytes.size();
            hdr.append(char(len & 0xFF));
            hdr.append(char((len >> 8) & 0xFF));
            hdr.append(char((len >> 16) & 0xFF));
            hdr.append(char((len >> 24) & 0xFF));

            if (!writeAll(COMPORT, hdr) || !writeAll(COMPORT, bytes)) { qDebug() << "[Images] send failed"; ok = false; break; }

            // wait for ESP ACK=50 before next image
            if (!waitForByte(char(50), 5000)) { qDebug() << "[Images] no ACK for" << path; ok = false; break; }
        }
    }

    if (ok) {
        if (!exitImageModeAndReset()) { qDebug() << "[Images] failed 49"; ok = false; }
        else qDebug() << "[Images] transfer complete; reset requested.";
    }
    return ok;
}
