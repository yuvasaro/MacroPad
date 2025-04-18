// mainwindow.h (refactored)
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QCloseEvent>
#include <QMessageBox>
#include <QQuickWidget>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#ifdef Q_OS_MAC
#include <Carbon/Carbon.h>
#elif __linux__
#include <X11/Xlib.h>
#endif

#include "profile.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static Profile* profileManager;
    Q_INVOKABLE void callHotkeyHandler(Profile* profile, int keyNum, const QString& type, const QString& content);


protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void showWindow();
    void exitApplication();
    void toggleDockIcon(bool show);

private:
    void createTrayIcon();

    QQuickWidget *qmlWidget;
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;

#ifdef __linux__
    Display *display;
#endif
};

#endif // MAINWINDOW_H
