#include "mainwindow.h"
#include "fileio.h"
#include "QApplication"
#include <QQmlEngine>
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

    setWindowTitle("MacroPad - Configuration");

    qmlRegisterType<FileIO>("FileIO", 1, 0, "FileIO");

    // Create QQuickWidget to display QML
    qmlWidget = new QQuickWidget(this);
    qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    qmlWidget->engine()->rootContext()->setContextProperty("profileManager", new Profile(this));
    qmlWidget->engine()->rootContext()->setContextProperty("fileIO", new FileIO(this));

    qmlWidget->setSource(QUrl("qrc:/Main.qml"));

    setCentralWidget(qmlWidget);

    createTrayIcon();
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
        toggleDockIcon(false);
    }
}

void MainWindow::showWindow() {
    toggleDockIcon(true);
    showNormal();  // Restore window
    activateWindow();
}

void MainWindow::exitApplication() {
    trayIcon->hide();  // Hide tray icon before quitting
    QApplication::quit();
}

// Toggle Dock Icon on macOS
void MainWindow::toggleDockIcon(bool show) {
#ifdef Q_OS_MAC
    ProcessSerialNumber psn = { 0, kCurrentProcess };
    if (show) {
        TransformProcessType(&psn, kProcessTransformToForegroundApplication);
    } else {
        TransformProcessType(&psn, kProcessTransformToUIElementApplication);
    }
#endif
}

// ===== WINDOWS IMPLEMENTATION =====

#ifdef _WIN32

//std::string path = "C:\\Users\\aarav\\OneDrive\\Desktop\\Arduino IDE.lnk";
std::string path = "Notepad";
std::wstring wpath(path.begin(), path.end());  // Convert std::string to std::wstring


std::unordered_map<UINT, std::function<void()>> MainWindow::hotkeyActions;
std::unique_ptr<Profile> currentProfile = std::make_unique<Profile>("DefaultProfile");
HHOOK keyboardHook = NULL;

//Week 6: modified
// KeyCustomization function: This callback function processes keyboard input for the global hotkeys
// LRESULT CALLBACK MainWindow::hotkeyCallback(int nCode, WPARAM wParam, LPARAM lParam) {
//     if (nCode == HC_ACTION) {
//         KBDLLHOOKSTRUCT* kbdStruct = (KBDLLHOOKSTRUCT*)lParam;
//         if (wParam == WM_KEYDOWN) {
//             auto it = hotkeyActions.find(kbdStruct->vkCode);
//             if (it != hotkeyActions.end()) {
//                 std::thread(it->second).detach();
//             // } else {
//             //     // If key is mapped to a macro number, execute it
//             //     auto macroIt = keyToMacroNumber.find(kbdStruct->vkCode);
//             //     if (macroIt != keyToMacroNumber.end()) {
//             //         int macroNum = macroIt->second;
//             //         hotkeyCallback(macroNum);
//             //     }
//             }
//         }
//     }
//     return CallNextHookEx(NULL, nCode, wParam, lParam);
// }

//week 8: new
LRESULT CALLBACK MainWindow::hotkeyCallback(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* kbdStruct = (KBDLLHOOKSTRUCT*)lParam;

        if (wParam == WM_KEYDOWN) {
            int vkCode = kbdStruct->vkCode;

            if (currentProfile) {
                // Check if a macro is assigned to this key
                std::unique_ptr<Macro>& macro = currentProfile->getMacro(vkCode);

                if (macro) { // If macro exists for the pressed key
                    QString type = macro->getType();
                    QString content = macro->getContent();

                    if (type == "execute") {
                        std::wstring wcontent = content.toStdWString();
                        ShellExecuteW(NULL, L"open", wcontent.c_str(), NULL, NULL, SW_SHOWNORMAL);
                    } else if (type == "simulate") {
                        QStringList keySequence = content.split(" ");

                        // Map string keys to virtual key codes
                        QMap<QString, int> keyMap = {
                            {"Cmd", VK_LWIN},   // Left Windows key
                            {"Shift", VK_SHIFT}, // Shift key
                            {"Ctrl", VK_CONTROL}, // Ctrl key
                            {"Alt", VK_MENU},    // Alt key
                            {"Space", VK_SPACE}, // Spacebar
                            {"Enter", VK_RETURN}, // Enter key
                            {"Backspace", VK_BACK}, // Backspace key
                            {"Tab", VK_TAB},      // Tab key
                            {"Esc", VK_ESCAPE},   // Escape key
                            {"1", '1'},           // Number key 1
                            {"2", '2'},           // Number key 2
                            {"3", '3'},           // Number key 3
                            {"4", '4'},           // Number key 4
                            {"5", '5'},           // Number key 5
                            // Add other necessary mappings here
                        };

                        // Simulate the key presses
                        Qt::KeyboardModifiers modifiers = Qt::NoModifier;

                        for (const QString& key : keySequence) {
                            if (keyMap.contains(key)) {
                                int vk = keyMap[key];

                                // Check for modifier keys like Shift, Cmd, etc.
                                if (vk == VK_SHIFT || vk == VK_LWIN || vk == VK_CONTROL || vk == VK_MENU) {
                                    modifiers |= Qt::KeyboardModifier(vk);
                                } else {
                                    // Send key event with modifier (if any)
                                    QKeyEvent keyPress(QEvent::KeyPress, vk, modifiers);
                                    QKeyEvent keyRelease(QEvent::KeyRelease, vk, modifiers);

                                    QApplication::postEvent(QApplication::focusWidget(), &keyPress);
                                    QApplication::postEvent(QApplication::focusWidget(), &keyRelease);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}


//week 8 keyboard version
// void MainWindow::registerGlobalHotkey(Profile* profile, int keyNum, const QString& type, const QString& content){
//     UINT vkCode = 0;

//     // Map keyNum to virtual key code (adjust mapping as needed)
//     switch (keyNum) {
//     case 1: vkCode = 0x31; break; //should be changed to macro keys after profile is loaded
//     case 2: vkCode = 0x32; break;
//     case 3: vkCode = 0x33; break;
//     case 4: vkCode = 0x34; break;
//     case 5: vkCode = 0x35; break;
//     case 6: vkCode = 0x36; break;
//     case 7: vkCode = 0x37; break;
//     case 8: vkCode = 0x38; break;
//     case 9: vkCode = 0x39; break;
//     default:
//         std::cerr << "Invalid key number specified.\n";
//         return;
//     }

//     // Register the hotkey based on the type
//     if (type == "execute") {
//         std::wstring wcontent(content.begin(), content.end());
//         RegisterHotkey(vkCode, [wcontent]() {
//             ShellExecuteW(NULL, L"open", wcontent.c_str(), NULL, NULL, SW_SHOWNORMAL);
//         });
//     } else if (type == "simulate") {
//         RegisterHotkey(vkCode, []() {
//             std::thread([]() {
//                 simulateAltSpace();  // Example simulation, customize as needed
//             }).detach();
//         });
//     } else {
//         std::cerr << "Unsupported action type.\n";
//     }

//     // Ensure the keyboard hook is set
//     if (!keyboardHook) {
//         keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, hotkeyCallback, GetModuleHandle(NULL), 0);
//     }
// }

//week 8: mapped to macros
void MainWindow::registerGlobalHotkey(Profile* profile, int keyNum, const QString& type, const QString& content) {
    if (!profile) {
        std::cerr << "Invalid profile.\n";
        return;
    }

    // Assign macro to the profile
    profile->setMacro(keyNum, type, content);
    currentProfile = std::make_unique<Profile>(profile);

    // Set global keyboard hook if not already set
    if (!keyboardHook) {
        keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, hotkeyCallback, GetModuleHandle(NULL), 0);
        if (!keyboardHook) {
            std::cerr << "Failed to set keyboard hook.\n";
        }
    }
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

        const QString& type = macro->getType();
        const QString& content = macro->getContent();

        if (macro->getType() == "keystroke") {

        } else if (macro->getType() == "program") {
            if (isAppBundle(content)) {
                QProcess::startDetached("open", {"-a", content});
            } else {
                QProcess::startDetached(content);
            }
        }
    }

    return noErr;
}

void MainWindow::registerGlobalHotkey(Profile* profile, int keyNum, const QString& type, const QString& content) {
    EventTypeSpec eventType;
    eventType.eventClass = kEventClassKeyboard;
    eventType.eventKind = kEventHotKeyPressed;

    EventHotKeyRef hotkeyRef;
    EventHotKeyID hotkeyID;
    hotkeyID.id = keyNum;

    // Create the event handler
    eventHandlerUPP = NewEventHandlerUPP(hotkeyCallback);
    InstallApplicationEventHandler(eventHandlerUPP, 1, &eventType, nullptr, nullptr);

    OSStatus status = RegisterEventHotKey(keyMap.at(keyNum), 0, hotkeyID, GetApplicationEventTarget(), 0, &hotkeyRef);

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
