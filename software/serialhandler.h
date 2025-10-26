#ifndef SERIALHANDLER_H
#define SERIALHANDLER_H

#include <QMainWindow>
#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QMessageBox>
#include <QString>
#include <QDebug>
#include <QTimer>

class SerialHandler : public QObject
{
    Q_OBJECT
public:
    explicit SerialHandler(QObject *parent = nullptr);
    ~SerialHandler();

    void sendProfile(int profileCode);

    bool enterImageMode();
    bool sendImageSlot(quint8 profileX, quint8 iconY, const QByteArray &pngBytes);
    bool exitImageModeAndReset();

    bool transferAllImages(const QString &root = "images", int profiles = 6, int iconsPerProfile = 6);
    //bool exportHeaders(const QString &root, const QString &outDir, int profiles = 6, int iconsPerProfile = 6) const;

signals:
    //void buttonPressed(int button);
    void dataReceived(const int number);
    void connectedChanged(bool isConnected);

private slots:
    void readSerialData();
    void tryConnect();
    void heartbeat();

private:
    QSerialPort* COMPORT=nullptr;
    QByteArray buffer;

    bool openPort(const QSerialPortInfo &info);
    void closePort();
    bool isCandidate(const QSerialPortInfo &info) const;
    void startScanning();
    void stopScanning();
    static bool writeAll(QSerialPort *port, const QByteArray &data, int chunk = 512, int timeoutMs = 4000);

    static QString pngPathFor(int x, int y, const QString &root, bool oneBasedY = true);
    static bool isPng(const QByteArray &bytes);
    //static QString cArrayFor(const QByteArray &bytes, const QString &symnbol);
    //static QString headerGuard(const QString &name);

    QTimer scanTimer;
    QTimer heartbeatTimer;
};

#endif // SERIALHANDLER_H
