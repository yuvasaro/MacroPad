#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QCloseEvent>
#include <QMessageBox>

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

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

#ifdef _WIN32
    void RegisterHotkey(UINT vkCode, std::function<void()> action);
    void doTasks(std::vector<INPUT>& inputs);
    static std::unordered_map<UINT, std::function<void()>> hotkeyActions;
    static void hotkeyCallback(int keyNum);

#endif

protected:
    void closeEvent(QCloseEvent *event) override;  // Override close event to minimize to tray

private slots:
    void showWindow();  // Restore window from system tray
    void exitApplication();  // Quit application

private:
    void registerGlobalHotkey();
    void createTrayIcon();  // System tray setup
   // static std::unordered_map<UINT, std::function<void()>> hotkeyActions;


QSystemTrayIcon *trayIcon;
QMenu *trayMenu;

#ifdef _WIN32
    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK KeyCustomization(int nCode, WPARAM wParam, LPARAM lParam);
    void SimulateKeystrokes(std::vector<INPUT>& inputs);
    static void OpenPath(const std::wstring& path);
    static HHOOK keyboardHook;
    static void simulateAltSpace();


#elif __APPLE__
    static OSStatus hotkeyCallback(EventHandlerCallRef nextHandler, EventRef event, void *userData);
#elif __linux__
    static void listenForHotkeys();
    Display *display;
#endif
};

#endif // MAINWINDOW_H
