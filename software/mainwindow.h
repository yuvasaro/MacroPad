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

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include "shellapi.h"
#elif __APPLE__
#include <Carbon/Carbon.h>
#elif __linux__
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#endif

#include "profile.h"
#include "apptracker.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT
    // Q_PROPERTY(QList<QObject*> profiles READ profiles NOTIFY profilesChanged) // Property allows QML to access profiles list

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // QList<QObject*> getProfiles() const; // getter so QML can access profiles list

#ifdef _WIN32
    void doTasks(std::vector<INPUT>& inputs);
    static std::unordered_map<UINT, std::function<void()>> hotkeyActions;

#endif

Q_INVOKABLE void registerGlobalHotkey(Profile* profile, int keyNum, const QString& type, const QString& content);

protected:
    void closeEvent(QCloseEvent *event) override;  // Override close event to minimize to tray

private slots:
    void showWindow();  // Restore window from system tray
    void exitApplication();  // Quit application
    void toggleDockIcon(bool show);
private:
    void createTrayIcon();
    static Profile* profileManager;

    AppTracker appTracker;

    //QList<QObject*> profiles;
    //QObject* currentProfile;

    Profile* profiles[6];
    Profile* currentProfile;


    void initializeProfiles();
    void switchCurrentProfile(const QString& appName);

   // QSharedPointer<Profile> currentProfile;
   // QList<QSharedPointer<Profile>> profiles;

QQuickWidget *qmlWidget;
QSystemTrayIcon *trayIcon;
QMenu *trayMenu;

#ifdef _WIN32
    static LRESULT CALLBACK hotkeyCallback(int nCode, WPARAM wParam, LPARAM lParam);
    static HHOOK keyboardHook;
#elif __APPLE__
    static OSStatus hotkeyCallback(EventHandlerCallRef nextHandler, EventRef event, void *userData);
#elif __linux__
    static void listenForHotkeys();
    Display *display;
#endif

};
#endif // MAINWINDOW_H
