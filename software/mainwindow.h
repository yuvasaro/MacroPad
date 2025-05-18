// mainwindow.h (refactored)
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "apptracker.h"
#include "serialhandler.h"
#include "fileio.h"
#include "profile.h"
#include "hotkeyhandler.h"
#include "knobhandler.h"
#include "serialhandler.h"
#include "apptracker.h"
#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QCloseEvent>
#include <QMessageBox>
#include <QQuickWidget>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlListProperty>

#ifdef Q_OS_MAC
#include <Carbon/Carbon.h>
#elif __linux__
#include <X11/Xlib.h>
#endif

#include "profile.h"
#include "apptracker.h"
#include "hotkeyhandler.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    Profile* getProfileInstance() { return profileInstance; };
    void setProfileInstance(Profile* profile);

    static Profile* profileManager;
    Q_INVOKABLE void callHotkeyHandler(Profile* profile, int keyNum, const QString& type, const QString& content);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onDataReceived(int number);

    void showWindow();
    void exitApplication();
    void toggleDockIcon(bool show);


private:
    void createTrayIcon();

    Profile* profileInstance;
    AppTracker appTracker;
    HotkeyHandler* hotkeyHandler;

    QQuickWidget *qmlWidget;
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    SerialHandler *m_serialHandler;


#ifdef __linux__
    Display *display;
#endif
};
#endif // MAINWINDOW_H
