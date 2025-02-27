#include "mainwindow.h"
#include "QApplication"
#include "QIcon"
#include <QAction>
#include <QMenu>
#include <iostream>
#include "profile.h"
#include "string"


#ifdef _WIN32

HHOOK MainWindow::keyboardHook = nullptr;
#endif

static Profile profile;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), trayIcon(new QSystemTrayIcon(this)), trayMenu(new QMenu(this)) {

    registerGlobalHotkey(&profile, 1, "program", "/Applications/Discord.app");  // This will set the keyboard hook properly
    createTrayIcon();

    setWindowTitle("Configuration Software");
}


MainWindow::~MainWindow() {
    /* if (keyboardHook) {
        UnhookWindowsHookEx(keyboardHook);
        keyboardHook = nullptr;
    } */
}

void MainWindow::createTrayIcon() {
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::warning(this, "Warning", "System tray is not available!");
        return;
    }

    // Set a valid icon (adjust path accordingly)
    trayIcon->setIcon(QIcon(":/icons/app_icon.png"));
    trayIcon->setToolTip("Configuration Software Running");

    // Create tray menu actions
    QAction *restoreAction = new QAction("Show Window", this);
    QAction *exitAction = new QAction("Exit", this);

    connect(restoreAction, &QAction::triggered, this, &MainWindow::showWindow);
    connect(exitAction, &QAction::triggered, this, &MainWindow::exitApplication);

    // Add actions to menu
    trayMenu->addAction(restoreAction);
    trayMenu->addAction(exitAction);

    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();
}


void MainWindow::closeEvent(QCloseEvent *event) {
    if (trayIcon->isVisible()) {
        hide();  // Hide the window
        event->ignore();  // Ignore the close event
    }
}

void MainWindow::showWindow() {
    showNormal();  // Restore window
    activateWindow();
}

void MainWindow::exitApplication() {
    trayIcon->hide();  // Hide tray icon before quitting
    QApplication::quit();
}
// ===== WINDOWS IMPLEMENTATION =====

#ifdef _WIN32
#include <thread>

//std::string path = "C:\\Users\\aarav\\OneDrive\\Desktop\\Arduino IDE.lnk";
std::string path = "Notepad";
std::wstring wpath(path.begin(), path.end());  // Convert std::string to std::wstring


std::unordered_map<UINT, std::function<void()>> MainWindow::hotkeyActions;
std::unique_ptr<Profile> currentProfile = std::make_unique<Profile>("DefaultProfile");


std::unordered_map<UINT, int> keyToMacroNumber = {
    {VK_NUMPAD1, 1}, {VK_NUMPAD2, 2}, {VK_NUMPAD3, 3},
    {VK_NUMPAD4, 4}, {VK_NUMPAD5, 5}, {VK_NUMPAD6, 6},
    {VK_NUMPAD7, 7}, {VK_NUMPAD8, 8}, {VK_NUMPAD9, 9}
};

//Week 6: created
// Function to get the config directory path
std::string getConfigDir() {
    PWSTR path = NULL;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &path))) {
        std::wstring wPath(path);
        CoTaskMemFree(path);  // Free memory allocated by SHGetKnownFolderPath
        return std::string(wPath.begin(), wPath.end()) + "\\YourAppName\\";
    }
    return "";
}

//Registers a hotkey and associates it with an action (in this case, a lambda function that performs an action).
void MainWindow::RegisterHotkey(UINT vkCode, std::function<void()> action) {
    hotkeyActions[vkCode] = action;
}

/*Customized arbitrary set of keystrokes
This is an example: Simulates pressing the Alt + Space keys, shoudl open up the system menu in Windows.
*/
void MainWindow::simulateAltSpace() {
    std::vector<INPUT> inputs(4);

    // Press ALT
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_MENU;

    // Press Space
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_SPACE;

    // Release Space
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = VK_SPACE;
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

    // Release ALT
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_MENU;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(inputs.size(), inputs.data(), sizeof(INPUT));
}


//Week 6: created
void MainWindow::hotkeyCallback(int keyNum) {
    if (currentProfile) {
        currentProfile->runMacro(keyNum);
    } else {
        std::cerr << "No active profile found!\n";
    }
}

//Week 6: modified
// KeyCustomization function: This callback function processes keyboard input for the global hotkeys
LRESULT CALLBACK MainWindow::KeyCustomization(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* kbdStruct = (KBDLLHOOKSTRUCT*)lParam;
        if (wParam == WM_KEYDOWN) {
            auto it = hotkeyActions.find(kbdStruct->vkCode);
            if (it != hotkeyActions.end()) {
                std::thread(it->second).detach();
            } else {
                // If key is mapped to a macro number, execute it
                auto macroIt = keyToMacroNumber.find(kbdStruct->vkCode);
                if (macroIt != keyToMacroNumber.end()) {
                    int macroNum = macroIt->second;
                    hotkeyCallback(macroNum);
                }
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}


// registerGlobalHotkey function: This function registers the global hotkeys (F6, F7)
void MainWindow::registerGlobalHotkey() {

    // Register F6 to open Notepad (or any executable defined in 'path')
    RegisterHotkey(VK_F6, []() {
        ShellExecuteW(NULL, L"open", wpath.c_str(), NULL, NULL, SW_SHOWNORMAL);
    });

    // Register F7 to simulate Ctrl+Alt
    RegisterHotkey(VK_F7, []() {
        std::thread([]() {
            // Simulate Control + Alt
            simulateAltSpace();
        }).detach();
    });

    // Set the keyboard hook to listen for key events globally (so the app runs)
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyCustomization, GetModuleHandle(NULL), 0);
}

#endif

// ===== MACOS IMPLEMENTATION =====
#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>
#include <QDebug>
#include <QProcess>
#include <QFileInfo>
#include <QDir>

static EventHandlerUPP eventHandlerUPP;

static const std::map<int, int> keyMap = {
    {1, kVK_ANSI_1},
    {2, kVK_ANSI_2},
    {3, kVK_ANSI_3},
    {4, kVK_ANSI_4},
    {5, kVK_ANSI_5},
    {6, kVK_ANSI_6},
    {7, kVK_ANSI_7},
    {8, kVK_ANSI_8},
    {9, kVK_ANSI_9}
};

bool isAppBundle(const QString &path) {
    QFileInfo appInfo(path);

    // 1. Check if path exists and is a directory
    if (!appInfo.exists() || !appInfo.isDir()) {
        return false;
    }

    // 2. Verify if it ends with ".app"
    if (!path.endsWith(".app", Qt::CaseInsensitive)) {
        return false;
    }

    // 3. Check if it contains an executable inside "Contents/MacOS/"
    QDir macOSDir(path + "/Contents/MacOS");
    QFileInfoList files = macOSDir.entryInfoList(QDir::Files | QDir::Executable);

    return !files.isEmpty();  // Returns true if there is at least one executable file
}

OSStatus MainWindow::hotkeyCallback(EventHandlerCallRef nextHandler, EventRef event, void *userData) {
    EventHotKeyID hotKeyID;
    GetEventParameter(event, kEventParamDirectObject, typeEventHotKeyID, NULL, sizeof(hotKeyID), NULL, &hotKeyID);
    std::unique_ptr<Macro>& macro = profile.getMacro(hotKeyID.id);

    if (macro != nullptr) {
        qDebug() << hotKeyID.id << "key pressed! Type:" << macro->getType() << "Content:" << macro->getContent();

        const std::string& type = macro->getType();
        const std::string& content = macro->getContent();
        QString contentQstr = QString::fromStdString(content);

        if (macro->getType() == "keystroke") {

        } else if (macro->getType() == "program") {
            if (isAppBundle(contentQstr)) {
                QProcess::startDetached("open", {"-a", contentQstr});
            } else {
                QProcess::startDetached(QString::fromStdString(content));
            }
        }
    }

    return noErr;
}

void MainWindow::registerGlobalHotkey(Profile* profile, int keyNum, const string& type, const string& content) {
    EventTypeSpec eventType;
    eventType.eventClass = kEventClassKeyboard;
    eventType.eventKind = kEventHotKeyPressed;

    EventHotKeyRef hotkeyRef;
    EventHotKeyID hotkeyID;
    hotkeyID.id = keyNum;

    // Create the event handler
    eventHandlerUPP = NewEventHandlerUPP(hotkeyCallback);
    InstallApplicationEventHandler(eventHandlerUPP, 1, &eventType, nullptr, nullptr);

<<<<<<< Updated upstream
<<<<<<< HEAD
    for (int i = 1; i <= 9; ++i) {
        EventHotKeyID hotKeyID;
        hotKeyID.signature = 'htk0' + i;
        hotKeyID.id = i;
=======
    // Register "Insert" key (kVK_Help is the closest macOS equivalent to Ins)
    OSStatus status_Ins = RegisterEventHotKey(kVK_ANSI_Grave, 0, hotKeyID_Ins, GetApplicationEventTarget(), 0, &hotKeyRef_Ins);
>>>>>>> 18b46333f0ff24c14be0f2ccfa01b087e87728c9
=======
    OSStatus status = RegisterEventHotKey(keyMap.at(keyNum), 0, hotkeyID, GetApplicationEventTarget(), 0, &hotkeyRef);
>>>>>>> Stashed changes

    if (status != noErr) {
        qDebug() << "Failed to register hotkey. Error code:" << status;
    } else {
        qDebug() << "Hotkey registered successfully!";
    }

    profile->setMacro(keyNum, type, content);
}
#endif


// ===== LINUX IMPLEMENTATION =====
#ifdef __linux__
#include <cstdlib>

void MainWindow::listenForHotkeys() {
    XEvent event;
    while (true) {
        XNextEvent(display, &event);
        if (event.type == KeyPress) {
            // Use 'notify-send' to show a notification without stealing focus
            system("notify-send 'MacroPad' 'Hotkey F5 Pressed!'");
        }
    }
}

void MainWindow::registerGlobalHotkey() {
    display = XOpenDisplay(NULL);
    if (!display) return;

    Window root = DefaultRootWindow(display);
    KeyCode keycode = XKeysymToKeycode(display, XK_F5);

    XGrabKey(display, keycode, AnyModifier, root, True, GrabModeAsync, GrabModeAsync);
    listenForHotkeys();
}
#endif
