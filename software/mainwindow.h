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
#include <QQmlListProperty>

#ifdef Q_OS_MAC
#include <Carbon/Carbon.h>
#elif __linux__
#include <X11/Xlib.h>
#endif

#include "profile.h"
#include "apptracker.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT
    // expose
    Q_PROPERTY(QQmlListProperty<Profile> profiles READ getProfiles NOTIFY profilesChanged)
    Q_PROPERTY(Profile* profileInstance READ getProfileInstance WRITE setProfileInstance NOTIFY profileInstanceChanged)

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    Q_INVOKABLE QQmlListProperty<Profile> getProfiles();

    static qsizetype profileCount(QQmlListProperty<Profile> *list);
    static Profile* profileAt(QQmlListProperty<Profile> *list, qsizetype index);
    Profile* getProfileInstance() { return profileInstance; };
    void setProfileInstance(Profile* profile);

    static Profile* profileManager;
    Q_INVOKABLE void callHotkeyHandler(Profile* profile, int keyNum, const QString& type, const QString& content);

signals:
    void profilesChanged();
    void profileInstanceChanged();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void showWindow();
    void exitApplication();
    void toggleDockIcon(bool show);

private:
    void createTrayIcon();

    Profile* profileInstance;
    AppTracker appTracker;


    QList<Profile*> profiles;
    Profile* currentProfile;


    void initializeProfiles();
    void switchCurrentProfile(const QString& appName);

    QQuickWidget *qmlWidget;
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;


#ifdef __linux__
    Display *display;
#endif
};
#endif // MAINWINDOW_H
