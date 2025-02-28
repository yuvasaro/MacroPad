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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), trayIcon(new QSystemTrayIcon(this)), trayMenu(new QMenu(this)) {

    Profile* profile = new Profile();

    registerGlobalHotkey(profile, 1, "execute", "C:\\Windows\\System32\\notepad.exe");
    registerGlobalHotkey(profile, 2, "simulate", "");

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
// void MainWindow::hotkeyCallback(int keyNum) {
//     if (currentProfile) {
//         currentProfile->runMacro(keyNum);
//     } else {
//         std::cerr << "No active profile found!\n";
//     }
// }



//Week 6: modified
// KeyCustomization function: This callback function processes keyboard input for the global hotkeys
LRESULT CALLBACK MainWindow::hotkeyCallback(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* kbdStruct = (KBDLLHOOKSTRUCT*)lParam;
        if (wParam == WM_KEYDOWN) {
            auto it = hotkeyActions.find(kbdStruct->vkCode);
            if (it != hotkeyActions.end()) {
                std::thread(it->second).detach();
            // } else {
            //     // If key is mapped to a macro number, execute it
            //     auto macroIt = keyToMacroNumber.find(kbdStruct->vkCode);
            //     if (macroIt != keyToMacroNumber.end()) {
            //         int macroNum = macroIt->second;
            //         hotkeyCallback(macroNum);
            //     }
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

//week 8
void MainWindow::registerGlobalHotkey(Profile* profile, int keyNum, const string& type, const string& content){
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
    if (type == "execute") {
        std::wstring wcontent(content.begin(), content.end());
        RegisterHotkey(vkCode, [wcontent]() {
            ShellExecuteW(NULL, L"open", wcontent.c_str(), NULL, NULL, SW_SHOWNORMAL);
        });
    } else if (type == "simulate") {
        RegisterHotkey(vkCode, []() {
            std::thread([]() {
                simulateAltSpace();  // Example simulation, customize as needed
            }).detach();
        });
    } else {
        std::cerr << "Unsupported action type.\n";
    }

    // Ensure the keyboard hook is set
    if (!keyboardHook) {
        keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, hotkeyCallback, GetModuleHandle(NULL), 0);
    }
}

// registerGlobalHotkey function: This function registers the global hotkeys (F6, F7)
//void MainWindow::registerGlobalHotkey() {

//     // Register F6 to open Notepad (or any executable defined in 'path')
//     RegisterHotkey(VK_F6, []() {
//         ShellExecuteW(NULL, L"open", wpath.c_str(), NULL, NULL, SW_SHOWNORMAL);
//     });

//     // Register F7 to simulate Ctrl+Alt
//     RegisterHotkey(VK_F7, []() {
//         std::thread([]() {
//             // Simulate Control + Alt
//             simulateAltSpace();
//         }).detach();
//     });

//     // Set the keyboard hook to listen for key events globally (so the app runs)
//     keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyCustomization, GetModuleHandle(NULL), 0);
// }

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

    if (hotKeyID.id == 1) {
        qDebug() << "Insert (Ins) key pressed! Opening Discord...";
        system("open -a 'Discord'");
    }
    else if (hotKeyID.id == 2) {
        qDebug() << "Home key pressed! Running executable at:" << EXECUTABLE_PATH;
        if (!QProcess::startDetached(EXECUTABLE_PATH)) {
            qDebug() << "Failed to launch executable!";
        }
    }

    return noErr;
}

void MainWindow::registerGlobalHotkey() {
    qDebug() << "Registering Insert (Ins) and Home keys as global hotkeys...";

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
    OSStatus status_Ins = RegisterEventHotKey(kVK_ANSI_Grave, 0, hotKeyID_Ins, GetApplicationEventTarget(), 0, &hotKeyRef_Ins);

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
