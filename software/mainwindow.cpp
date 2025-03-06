#include "mainwindow.h"
#include "QApplication"
#include "QIcon"
#include <QAction>
#include <QMenu>
#include "string"
#include <QQuickItem>
#include <QQmlContext>
#include <QDebug>


#ifdef _WIN32
#include "shellapi.h"
HHOOK MainWindow::keyboardHook = nullptr;
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), trayIcon(new QSystemTrayIcon(this)), trayMenu(new QMenu(this)) {
    registerGlobalHotkey(&profile, 1, "program", "/Applications/Discord.app");

<<<<<<< Updated upstream
    registerGlobalHotkey();  // This will set the keyboard hook properly
=======
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
    Profile *profileManager = new Profile(this);


    qmlRegisterType<FileIO>("FileIO", 1, 0, "FileIO");
    qmlRegisterType<Macro>("Macro", 1, 0, "Macro");
    qmlRegisterType<Profile>("Profile", 1, 0, "Profile");


    // Register with QML
    qmlWidget->engine()->rootContext()->setContextProperty("fileIO", fileIO);
    qmlWidget->engine()->rootContext()->setContextProperty("Macro", macro);
    qmlWidget->engine()->rootContext()->setContextProperty("profileInstance", profileManager);



    qmlWidget->setSource(QUrl("qrc:/Main.qml"));

    QObject *root = qmlWidget->rootObject();
    if (root) {
        QObject *profileObj = root->findChild<QObject*>("profileManager");
        if (profileObj) {
            connect(profileObj, SIGNAL(keyConfigured(int,QString,QString)),
                    this, SLOT(onKeyConfigured(int,QString,QString)));
        }
    }

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    layout->addWidget(qmlWidget);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

>>>>>>> Stashed changes
    createTrayIcon();

    setWindowTitle("Configuration Software");
}

MainWindow::~MainWindow() {
    if (keyboardHook) {
        UnhookWindowsHookEx(keyboardHook);
        keyboardHook = nullptr;
    }
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


// KeyCustomization function: This callback function processes keyboard input for the global hotkeys
LRESULT CALLBACK MainWindow::KeyCustomization(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* kbdStruct = (KBDLLHOOKSTRUCT*)lParam;
        if (wParam == WM_KEYDOWN) {
            auto it = hotkeyActions.find(kbdStruct->vkCode);
            if (it != hotkeyActions.end()) {
                std::thread(it->second).detach();  // Run the registered action in a separate thread
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

static EventHotKeyRef hotKeyRef_Ins;
static EventHotKeyRef hotKeyRef_Home;
static EventHotKeyID hotKeyID_Ins;
static EventHotKeyID hotKeyID_Home;
static EventHandlerUPP eventHandlerUPP;

// Placeholder path to the executable
const QString EXECUTABLE_PATH = "/Users/yuvasaro/Developer/C/experiments/bits/swap/inplace_swap";  // Replace this!

OSStatus MainWindow::hotkeyCallback(EventHandlerCallRef nextHandler, EventRef event, void *userData) {
    EventHotKeyID hotKeyID;
    GetEventParameter(event, kEventParamDirectObject, typeEventHotKeyID, NULL, sizeof(hotKeyID), NULL, &hotKeyID);
<<<<<<< Updated upstream

    if (hotKeyID.id == 1) {
        qDebug() << "Insert (Ins) key pressed! Opening Discord...";
        system("open -a 'Discord'");
    }
    else if (hotKeyID.id == 2) {
        qDebug() << "Home key pressed! Running executable at:" << EXECUTABLE_PATH;
        if (!QProcess::startDetached(EXECUTABLE_PATH)) {
            qDebug() << "Failed to launch executable!";
=======
    QSharedPointer<Macro> macro = profile.getMacro(hotKeyID.id);
    if (!macro.isNull()) {
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
>>>>>>> Stashed changes
        }
    }

    return noErr;
}

<<<<<<< Updated upstream
void MainWindow::registerGlobalHotkey() {
    qDebug() << "Registering Insert (Ins) and Home keys as global hotkeys...";

=======


void MainWindow::onKeyConfigured(int keyIndex, const QString &type, const QString &content) {
    qDebug() << "Registering hotkey for keyIndex:" << keyIndex << "Type:" << type << "Content:" << content;
    registerGlobalHotkey(profileManager, keyIndex, type, content);
}

void MainWindow::registerGlobalHotkey(Profile* profile, int keyNum, const QString& type, const QString& content) {
>>>>>>> Stashed changes
    EventTypeSpec eventType;
    eventType.eventClass = kEventClassKeyboard;
    eventType.eventKind = kEventHotKeyPressed;

    hotKeyID_Ins.signature = 'htk1';
    hotKeyID_Ins.id = 1;
    hotKeyID_Home.signature = 'htk2';
    hotKeyID_Home.id = 2;

    // Create the event handler
    eventHandlerUPP = NewEventHandlerUPP(hotkeyCallback);
    InstallApplicationEventHandler(eventHandlerUPP, 1, &eventType, nullptr, nullptr);

    // Register "Insert" key (kVK_Help is the closest macOS equivalent to Ins)
    OSStatus status_Ins = RegisterEventHotKey(kVK_Help, 0, hotKeyID_Ins, GetApplicationEventTarget(), 0, &hotKeyRef_Ins);

    // Register "Home" key
    OSStatus status_Home = RegisterEventHotKey(kVK_Home, 0, hotKeyID_Home, GetApplicationEventTarget(), 0, &hotKeyRef_Home);

    if (status_Ins != noErr) {
        qDebug() << "Failed to register Insert hotkey. Error code:" << status_Ins;
    } else {
        qDebug() << "Insert (Ins) hotkey registered successfully!";
    }

    if (status_Home != noErr) {
        qDebug() << "Failed to register Home hotkey. Error code:" << status_Home;
    } else {
        qDebug() << "Home hotkey registered successfully! Press Home to run the executable.";
    }
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
