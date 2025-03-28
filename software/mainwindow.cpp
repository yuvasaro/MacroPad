#include "mainwindow.h"
#include "config.h"
#include "fileio.h"
#include "QApplication"
#include <QQmlEngine>
#include "QIcon"
#include <QAction>
#include <QVBoxLayout>
#include <QMenu>
#include <QQuickItem>
#include <iostream>
#include <thread>
#include "profile.h"
#include "string"
#include <objc/objc.h>
#include <objc/NSObject.h>

#ifdef _WIN32

HHOOK MainWindow::keyboardHook = nullptr;
#endif

Profile* MainWindow::profileManager = new Profile(NULL);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), trayIcon(new QSystemTrayIcon(this)), trayMenu(new QMenu(this)) {

#ifdef _WIN32 //windows demostration

    registerGlobalHotkey(&profile, 1, "executable", "Notepad");
    registerGlobalHotkey(&profile, 2, "keystroke", "Ctrl+Alt+Tab");
    registerGlobalHotkey(&profile, 3, "executable", "file:///C:/Program Files/BlueJ/BlueJ.exe");

    qDebug() << "Profile 'TestProfile' created and saved.";

    // Print out the macros in the profile for debugging
    qDebug() << "Assigned macros for 'TestProfile':";
    for (int i = 1; i <= 9; ++i) { // assuming you only have up to 5 macro keys
        std::unique_ptr<Macro>& macro = profile.getMacro(i);
        if (macro) {
            qDebug() << "Key " << i << " -> Type:" << macro->getType() << ", Content:" << macro->getContent();
        } else {
            qDebug() << "Key " << i << " is not assigned a macro.";
        }
    }

#endif

    setWindowTitle("MacroPad - Configuration");

    // Create QQuickWidget to display QML
    qmlWidget = new QQuickWidget(this);
    qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    FileIO *fileIO = new FileIO(this);
    Macro *macro = new Macro(this);

    qmlRegisterType<FileIO>("FileIO", 1, 0, "FileIO");
    qmlRegisterType<Macro>("Macro", 1, 0, "Macro");

    // Register with QML
    qmlWidget->engine()->rootContext()->setContextProperty("fileIO", fileIO);
    qmlWidget->engine()->rootContext()->setContextProperty("Macro", macro);
    qmlWidget->engine()->rootContext()->setContextProperty("profileInstance", profileManager);
    qmlWidget->engine()->rootContext()->setContextProperty("mainWindow", this);
    qmlWidget->setSource(QUrl("qrc:/Main.qml"));

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    layout->addWidget(qmlWidget);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    createTrayIcon();

    registerGlobalHotkey(profileManager, 1, "executable", "/Applications/Discord.app");
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
#ifdef Q_OS_MAC
    QString iconPath = QCoreApplication::applicationDirPath() + "/../Resources/MPIcon.png";
    QIcon icon(iconPath);
    // qDebug() << "Loading tray icon from:" << iconPath;
    // qDebug() << "File exists:" << QFile::exists(iconPath);
    // qDebug() << "Icon loaded successfully:" << !icon.isNull();
    trayIcon->setIcon(icon);
#endif

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
// std::string path = "Notepad";
//std::wstring wpath(path.begin(), path.end());  // Convert std::string to std::wstring


std::unordered_map<UINT, std::function<void()>> MainWindow::hotkeyActions;
std::unique_ptr<Profile> currentProfile = std::make_unique<Profile>("DefaultProfile");
HHOOK keyboardHook = NULL;

// //Week 6: modified

LRESULT CALLBACK MainWindow::hotkeyCallback(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* kbdStruct = (KBDLLHOOKSTRUCT*)lParam;

        if (wParam == WM_KEYDOWN) {
            int vkCode = kbdStruct->vkCode;

            // Check if the key has a registered action
            auto it = hotkeyActions.find(vkCode);
            if (it != hotkeyActions.end()) {
                it->second(); // Execute the stored action (macro)
                return 1;  // Prevents default key behavior (optional)
            }
        }
    }

    // Pass the event to the next hook in the chain
    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

//week 8 keyboard version
void MainWindow::registerGlobalHotkey(Profile* profile, int keyNum, const QString& type, const QString& content){
    UINT vkCode = 0;

    // Map keyNum to virtual key code (adjust mapping as needed)
    switch (keyNum) {
    case 1: vkCode = 0x31; break; //should be changed to macro keys after profile is loaded
    case 2: vkCode = 0x32; break;
    case 3: vkCode = 0x33; break;
    case 4: vkCode = 0x34; break;
    case 5: vkCode = 0x35; break;
    case 6: vkCode = 0x36; break;
    case 7: vkCode = 0x37; break;
    case 8: vkCode = 0x38; break;
    case 9: vkCode = 0x39; break;
    default:
        std::cerr << "Invalid key number specified.\n";
        return;
    }

    // Register the hotkey based on the type
    if (type == "executable") {
        std::wstring wcontent = content.toStdWString();
        hotkeyActions[vkCode] = [wcontent]() {
            ShellExecuteW(NULL, L"open", wcontent.c_str(), NULL, NULL, SW_SHOWNORMAL);
        };
    } else if (type == "keystroke") {
        hotkeyActions[vkCode] = [content]() {
            std::thread([content]() {

                // Define key mapping
                QMap<QString, int> keyMap = {
                    {"Cmd", VK_LWIN}, {"Shift", VK_SHIFT}, {"Ctrl", VK_CONTROL}, {"Alt", VK_MENU},
                    {"Space", VK_SPACE}, {"Enter", VK_RETURN}, {"Backspace", VK_BACK}, {"Tab", VK_TAB},
                    {"Esc", VK_ESCAPE}
                };

                for (char c = '0'; c <= '9'; ++c) {
                    keyMap[QString(c)] = c;
                }

                for (char c = 'A'; c <= 'Z'; ++c) {
                    keyMap[QString(c)] = c;
                }

                // Parse the key sequence
                QStringList keySequence = content.split("+");
                std::vector<int> keyCodes;

                for (const QString& key : std::as_const(keySequence)) {
                    if (keyMap.contains(key)) {
                        keyCodes.push_back(keyMap[key]);
                    }
                }

                // Press all keys
                for (int key : keyCodes) {
                    keybd_event(key, 0, 0, 0);
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Small delay

                // Release all keys (in reverse order)
                for (auto it = keyCodes.rbegin(); it != keyCodes.rend(); ++it) {
                    keybd_event(*it, 0, KEYEVENTF_KEYUP, 0);
                }
            }).detach();
        };
    } else {
        std::cerr << "Unsupported action type.\n";
    }

    // Ensure the keyboard hook is set
    if (!keyboardHook) {
        keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, hotkeyCallback, GetModuleHandle(NULL), 0);
    }

    profile->setMacro(keyNum, type, content);
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
#include <QThread>

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

static const QMap<QString, CGKeyCode> keyHex{
    {"cmd", 0x31},    // Command
    {"shift", 0x38},  // Shift
    {"ctrl", 0x3B},   // Control
    {"alt", 0x3A},    // Option
    {"space", 0x31},  // Space
    {"a", 0x00},      // 'a'
    {"b", 0x0B},      // 'b'
    {"c", 0x08},      // 'c'
    {"d", 0x02},      // 'd'
    {"e", 0x04},      // 'e'
    {"f", 0x05},      // 'f'
    {"g", 0x03},      // 'g'
    {"h", 0x04},      // 'h'
    {"i", 0x22},      // 'i'
    {"j", 0x26},      // 'j'
    {"k", 0x28},      // 'k'
    {"l", 0x25},      // 'l'
    {"m", 0x2E},      // 'm'
    {"n", 0x2D},      // 'n'
    {"o", 0x1F},      // 'o'
    {"p", 0x23},      // 'p'
    {"q", 0x0C},      // 'q'
    {"r", 0x0F},      // 'r'
    {"s", 0x01},      // 's'
    {"t", 0x11},      // 't'
    {"u", 0x20},      // 'u'
    {"v", 0x09},      // 'v'
    {"w", 0x0D},      // 'w'
    {"x", 0x07},      // 'x'
    {"y", 0x10},      // 'y'
    {"z", 0x06},      // 'z'
    {"1", 0x12},      // '1'
    {"2", 0x13},      // '2'
    {"3", 0x14},      // '3'
    {"4", 0x15},      // '4'
    {"5", 0x17},      // '5'
    {"6", 0x16},      // '6'
    {"7", 0x1A},      // '7'
    {"8", 0x1C},      // '8'
    {"9", 0x19},      // '9'
    {"0", 0x1D}       // '0'
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

void pressAndReleaseKey(const QList<QString>& keys) {
    QList<CGEventRef> keysDown;
    QList<CGEventRef> keysUp;
    CGEventFlags flags = 0;

    // Iterate over the provided keys to determine modifiers and normal keys
    for (const auto& key : keys) {
        if (key == "cmd") {
            flags |= kCGEventFlagMaskCommand;  // Add Command modifier
        } else if (key == "shift") {
            flags |= kCGEventFlagMaskShift;  // Add Shift modifier
        } else if (key == "ctrl") {
            flags |= kCGEventFlagMaskControl; // Add Control modifier
        } else if (key == "alt") {
            flags |= kCGEventFlagMaskAlternate; // Add Option (Alt) modifier
        } else {
            // Create keydown event for normal key
            CGEventRef keyDown = CGEventCreateKeyboardEvent(NULL, keyHex[key], true);
            CGEventSetFlags(keyDown, flags); // Apply modifier flags
            keysDown.push_back(keyDown);

            // Create keyup event for normal key
            CGEventRef keyUp = CGEventCreateKeyboardEvent(NULL, keyHex[key], false);
            CGEventSetFlags(keyUp, flags); // Apply modifier flags
            keysUp.push_back(keyUp);
        }
    }

    // Post keydown events
    for (CGEventRef keyDown : keysDown) {
        CGEventPost(kCGHIDEventTap, keyDown);
    }

    qDebug() << "Pressed";

    usleep(1000); // Sleep for 10ms to simulate key press duration

    // Post keyup events in reverse order (release main key first, then modifier)
    for (CGEventRef keyUp : keysUp) {
        CGEventPost(kCGHIDEventTap, keyUp);
    }

    qDebug() << "Released";

    // Release resources
    for (CGEventRef key : keysDown) {
        CFRelease(key);
    }
    for (CGEventRef key : keysUp) {
        CFRelease(key);
    }
}

bool simulateCommandSpace() {
    QList<QString> keys;
    keys.append("cmd");
    keys.append("shift");
    keys.append("4");

    pressAndReleaseKey(keys);
}

OSStatus MainWindow::hotkeyCallback(EventHandlerCallRef nextHandler, EventRef event, void *userData){
    EventHotKeyID hotKeyID;
    GetEventParameter(event, kEventParamDirectObject, typeEventHotKeyID, NULL, sizeof(hotKeyID), NULL, &hotKeyID);
    QSharedPointer<Macro> macro = profileManager->getMacro(hotKeyID.id);

    if (!macro.isNull()) {
        qDebug() << hotKeyID.id << "key pressed! Type:" << macro->getType() << "Content:" << macro->getContent();

        const QString& type = macro->getType();
        const QString& content = macro->getContent();

        if (macro->getType() == "keystroke") {

        } else if (macro->getType() == "executable") {
            if (isAppBundle(content)) {
                // QProcess::startDetached("open", {"-a", content});
                simulateCommandSpace();
                qDebug() << "Simulated command space";
            } else {
                QProcess::startDetached(content);
            }
        }
    }

    return noErr;
}

void MainWindow::registerGlobalHotkey(Profile* profile, int keyNum, const QString& type, const QString& content) {
    qDebug() << "registerGlobalHotkey called with:" << keyNum << type << content;

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

