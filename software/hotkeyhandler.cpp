// hotkeyhandler.cpp
#include "hotkeyhandler.h"
#include <QDir>
#include <QDebug>
#include <QMap>
#include <iostream>
#include <thread>
#include <chrono>
#include <QProcess>
#include <QFileInfo>
#include <QFileInfoList>


Profile* HotkeyHandler::profileManager = new Profile("General", "MacroPad", nullptr);

#ifdef __APPLE__
QMap<int, EventHotKeyRef> HotkeyHandler::registeredHotkeys;
#endif

#ifdef _WIN32
HHOOK HotkeyHandler::keyboardHook = nullptr;
std::unordered_map<UINT, std::function<void()>> HotkeyHandler::hotkeyActions;
std::wstring HotkeyHandler::wpathExec;

LRESULT CALLBACK HotkeyHandler::hotkeyCallback(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* kbdStruct = (KBDLLHOOKSTRUCT*)lParam;
        if (wParam == WM_KEYDOWN) {
            int vkCode = kbdStruct->vkCode;
            auto it = hotkeyActions.find(vkCode);
            if (it != hotkeyActions.end()) {
                OutputDebugStringW(L"Hotkey matched, executing action...\n");
                it->second();
                return 1;
            }
        }
    }
    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

void HotkeyHandler::executeHotkey(int hotKeyNum, Profile* profileInstance)
{
    if (!profileInstance) {
        qWarning() << "HotkeyHandler: no profile passed in!";
        return;
    }

    auto macro = profileInstance->getMacro(hotKeyNum);
    if (macro.isNull()) {
        qDebug() << "No macro for key" << hotKeyNum;
        return;
    }

    const QString type    = macro->getType();
    const QString content = macro->getContent();

    qDebug() << "Executing macro" << hotKeyNum << type << content;

    if (type == "keystroke") {
        std::thread([content]() {
            QMap<QString,int> keyMap = {
                {"Cmd", VK_LWIN}, {"Shift", VK_SHIFT},
                {"Ctrl", VK_CONTROL}, {"Alt", VK_MENU},
                {"Space", VK_SPACE}, {"Enter", VK_RETURN},
                {"Backspace", VK_BACK}, {"Tab", VK_TAB},
                {"Esc", VK_ESCAPE}
            };
            for (char c = '0'; c <= '9'; ++c) keyMap[QString(c)] = c;
            for (char c = 'A'; c <= 'Z'; ++c) keyMap[QString(c)] = c;

            auto keys = content.split("+");
            std::vector<int> codes;
            for (auto &k : keys)
                if (keyMap.contains(k))
                    codes.push_back(keyMap[k]);

            for (int kc : codes) keybd_event(kc, 0, 0, 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            for (auto it = codes.rbegin(); it != codes.rend(); ++it)
                keybd_event(*it, 0, KEYEVENTF_KEYUP, 0);
        }).detach();
    }
    else if (type == "executable") {
        QString fixed = content;
        if (fixed.startsWith("/")) fixed.remove(0,1);
        wpathExec = QDir::toNativeSeparators(fixed).toStdWString();
        ShellExecuteW(NULL, L"open", wpathExec.c_str(), NULL, NULL, SW_SHOWNORMAL);
    }
}

#endif

#ifdef __APPLE__
static EventHandlerUPP eventHandlerUPP;
static const std::map<int, int> keyMap = {
    {1, kVK_ANSI_1}, {2, kVK_ANSI_2}, {3, kVK_ANSI_3}, {4, kVK_ANSI_4}, {5, kVK_ANSI_5},
    {6, kVK_ANSI_6}, {7, kVK_ANSI_7}, {8, kVK_ANSI_8}, {9, kVK_ANSI_9}
};

bool isAppBundle(const QString &path) {
    QFileInfo appInfo(path);
    if (!appInfo.exists() || !appInfo.isDir()) return false;
    if (!path.endsWith(".app", Qt::CaseInsensitive)) return false;
    QDir macOSDir(path + "/Contents/MacOS");
    return !macOSDir.entryInfoList(QDir::Files | QDir::Executable).isEmpty();
}

OSStatus HotkeyHandler::hotkeyCallback(EventHandlerCallRef nextHandler, EventRef event, void *userData) {
    EventHotKeyID hotKeyID;
    GetEventParameter(event, kEventParamDirectObject, typeEventHotKeyID, NULL, sizeof(hotKeyID), NULL, &hotKeyID);
    QSharedPointer<Macro> macro = HotkeyHandler::profileManager->getMacro(hotKeyID.id);
    if (!macro.isNull()) {
        const QString& type = macro->getType();
        const QString& content = macro->getContent();
        if (type == "keystroke") {
            // Implement macOS keystroke logic here
        } else if (type == "executable") {
            if (isAppBundle(content)) {
                QProcess::startDetached("open", {"-a", content});
            } else {
                QProcess::startDetached(content);
            }
        }
    }
    return noErr;
}

void HotkeyHandler::executeHotkey(int hotKeyNum, Profile* profileInstance) {
    QSharedPointer<Macro> macro = profileInstance->getMacro(hotKeyNum);

    if (!macro.isNull()) {
        qDebug() << hotKeyNum << "key pressed! Type:" << macro->getType() << "Content:" << macro->getContent();

        const QString& type = macro->getType();
        const QString& content = macro->getContent();

        if (macro->getType() == "keystroke") {

        } else if (macro->getType() == "executable") {
            if (isAppBundle(content)) {
                QProcess::startDetached("open", {"-a", content});
            } else {
                QProcess::startDetached(content);
            }
        }
    }
}

#endif



#ifdef __linux__
Display* HotkeyHandler::display = nullptr;

void HotkeyHandler::listenForHotkeys() {
    XEvent event;
    while (true) {
        XNextEvent(display, &event);
        if (event.type == KeyPress) {
            system("notify-send 'MacroPad' 'Hotkey F5 Pressed!'");
        }
    }
}
#endif

void HotkeyHandler::registerGlobalHotkey(Profile* profile, int keyNum, const QString& type, const QString& content) {
#ifdef _WIN32
#ifdef DEBUG
    UINT vkCode = 0;
    switch (keyNum) {
    case 1: vkCode = 0x31; break;
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

    //Save the profile pointer for future runtime access
    HotkeyHandler::profileManager = profile;

    // Register a runtime lookup lambda
    hotkeyActions[vkCode] = [keyNum]() {
        if (!HotkeyHandler::profileManager) {
            OutputDebugStringW(L"No active profile.\n");
            return;
        }

        QSharedPointer<Macro> macro = HotkeyHandler::profileManager->getMacro(keyNum);
        if (macro.isNull()) {
            OutputDebugStringW(L"Macro not found.\n");
            return;
        }

        const QString type = macro->getType();
        const QString content = macro->getContent();

        if (type == "executable") {
            QString fixedPath = content;
            if (fixedPath.startsWith("/")) fixedPath = fixedPath.mid(1);
            std::wstring wcontent = QDir::toNativeSeparators(fixedPath).toStdWString();
            OutputDebugStringW((L"Launching executable: " + wcontent + L"\n").c_str());
            ShellExecuteW(NULL, L"open", wcontent.c_str(), NULL, NULL, SW_SHOWNORMAL);
        } else if (type == "keystroke") {
            std::thread([content]() {
                QMap<QString, int> keyMap = {
                    {"Cmd", VK_LWIN}, {"Shift", VK_SHIFT}, {"Ctrl", VK_CONTROL}, {"Alt", VK_MENU},
                    {"Space", VK_SPACE}, {"Enter", VK_RETURN}, {"Backspace", VK_BACK}, {"Tab", VK_TAB},
                    {"Esc", VK_ESCAPE}
                };
                for (char c = '0'; c <= '9'; ++c) keyMap[QString(c)] = c;
                for (char c = 'A'; c <= 'Z'; ++c) keyMap[QString(c)] = c;

                QStringList keySequence = content.split("+");
                std::vector<int> keyCodes;
                for (const QString& key : std::as_const(keySequence)) {
                    if (keyMap.contains(key)) keyCodes.push_back(keyMap[key]);
                }

                for (int key : keyCodes) keybd_event(key, 0, 0, 0);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                for (auto it = keyCodes.rbegin(); it != keyCodes.rend(); ++it)
                    keybd_event(*it, 0, KEYEVENTF_KEYUP, 0);
            }).detach();
        }
    };

    if (!keyboardHook) {
        keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, hotkeyCallback, GetModuleHandle(NULL), 0);
        if (!keyboardHook) {
            OutputDebugStringW(L"Failed to set keyboard hook.\n");
        } else {
            OutputDebugStringW(L"Keyboard hook set.\n");
        }
    }
#endif
    profile->setMacro(keyNum, type, content);
#elif __APPLE__
#ifdef DEBUG
    qDebug() << "registerGlobalHotkey called with:" << keyNum << type << content;

    // Check if already registered
    if (registeredHotkeys.contains(keyNum)) {
        qDebug() << "Hotkey already registered, unregistering first.";
        UnregisterEventHotKey(registeredHotkeys[keyNum]);
        registeredHotkeys.remove(keyNum);
    }

    EventTypeSpec eventType = { kEventClassKeyboard, kEventHotKeyPressed };
    EventHotKeyRef hotkeyRef;
    EventHotKeyID hotkeyID = { 0, static_cast<UInt32>(keyNum) };
    eventHandlerUPP = NewEventHandlerUPP(hotkeyCallback);
    InstallApplicationEventHandler(eventHandlerUPP, 1, &eventType, nullptr, nullptr);
    OSStatus status = RegisterEventHotKey(keyMap.at(keyNum), 0, hotkeyID, GetApplicationEventTarget(), 0, &hotkeyRef);
    if (status != noErr) {
        qDebug() << "Failed to register hotkey. Error code:" << status;
    } else {
        qDebug() << "Hotkey registered successfully!";
        registeredHotkeys[keyNum] = hotkeyRef;
    }
#endif
    profile->setMacro(keyNum, type, content);
#elif __linux__
    display = XOpenDisplay(NULL);
    if (!display) return;
    Window root = DefaultRootWindow(display);
    KeyCode keycode = XKeysymToKeycode(display, XK_F5);
    XGrabKey(display, keycode, AnyModifier, root, True, GrabModeAsync, GrabModeAsync);
    listenForHotkeys();
#endif
}

//key triggering behavior helper functions
/*
void HotkeyHandler::volumeUp()
{
#ifdef _WIN32
    qDebug() << "volumeUp called";
    INPUT inputs[2] = {};
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_VOLUME_UP;
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_VOLUME_UP;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(2, inputs, sizeof(INPUT));
#endif

#ifdef __APPLE__
    MainWindow::macVolume = (MainWindow::macVolume >= 100) ? MainWindow::macVolume : MainWindow::macVolume + 6;
    setSystemVolume(MainWindow::macVolume);
#endif
}

void HotkeyHandler:: volumeDown()
{
#ifdef _WIN32
    qDebug() << "volumeDown called";
    INPUT inputs[2] = {};
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_VOLUME_DOWN;
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_VOLUME_DOWN;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(2, inputs, sizeof(INPUT));
#endif

#ifdef __APPLE__
    MainWindow::macVolume = (MainWindow::macVolume <= 0) ? MainWindow::macVolume : MainWindow::macVolume - 6;
    setSystemVolume(MainWindow::macVolume);
#endif
}

void HotkeyHandler:: mute()
{
#ifdef _WIN32
    qDebug() << "mute called";
    INPUT inputs[2] = {};
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_VOLUME_MUTE;
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_VOLUME_MUTE;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(2, inputs, sizeof(INPUT));
#endif

#ifdef __APPLE__
    toggleMuteSystem();
#endif
}
*/

// Scroll functions

void HotkeyHandler:: scrollUp()
{
#ifdef _WIN32
    qDebug() << "scrollUp called on Windows";
    // Simulate one notch of wheel up (WHEEL_DELTA = +120)
    INPUT input = {};
    input.type             = INPUT_MOUSE;
    input.mi.dwFlags       = MOUSEEVENTF_WHEEL;
    input.mi.mouseData     = WHEEL_DELTA;
    SendInput(1, &input, sizeof(input));
#endif

#ifdef __APPLE__
    // qDebug() << "scrollUp called on macOS";
    // QScrollArea* scrollArea = ui->scrollArea;
    // QScrollBar* vScrollBar = scrollArea->verticalScrollBar();
    // if (vScrollBar) {
    //     vScrollBar->setValue(vScrollBar->value() - 50);
    // }
#endif
}

void HotkeyHandler:: scrollDown()
{
#ifdef _WIN32
    qDebug() << "scrollDown called on Windows";
    // Simulate one notch of wheel down (–120)
    INPUT input = {};
    input.type             = INPUT_MOUSE;
    input.mi.dwFlags       = MOUSEEVENTF_WHEEL;
    input.mi.mouseData     = -WHEEL_DELTA;
    SendInput(1, &input, sizeof(input));
#endif

#ifdef __APPLE__
    // qDebug() << "scrollDown called on macOS";
    // QScrollArea* scrollArea = ui->scrollArea;
    // QScrollBar* vScrollBar = scrollArea->verticalScrollBar();
    // if (vScrollBar) {
    //     vScrollBar->setValue(vScrollBar->value() + 50);
    // }
#endif
}

void HotkeyHandler::autoScrollToggle() {
#ifdef   _WIN32
    qDebug() << "AutoScrollToggle called";
    // two events: middle-button down, then up
    INPUT inputs[2] = {};

    // middle-button down
    inputs[0].type               = INPUT_MOUSE;
    inputs[0].mi.dwFlags         = MOUSEEVENTF_MIDDLEDOWN;

    // middle-button up
    inputs[1].type               = INPUT_MOUSE;
    inputs[1].mi.dwFlags         = MOUSEEVENTF_MIDDLEUP;

    SendInput(2, inputs, sizeof(INPUT));
#endif
}



#ifdef _WIN32
// Increase screen brightness by 10%
void HotkeyHandler::brightnessUp()
{
    qDebug() << "brightnessUp called";
    QProcess::execute("powershell", QStringList() << "-Command"
                                                  << "$b = (Get-WmiObject -Namespace root/wmi -Class WmiMonitorBrightness); "
                                                     "$c = (Get-WmiObject -Namespace root/wmi -Class WmiMonitorBrightnessMethods); "
                                                     "$level = $b.CurrentBrightness + 10; "
                                                     "if ($level -gt 100) { $level = 100 }; "
                                                     "$c.WmiSetBrightness(1, $level)");
}

void HotkeyHandler::brightnessDown()
{
    qDebug() << "brightnessDown called";
    QProcess::execute("powershell", QStringList() << "-Command"
                                                  << "$b = (Get-WmiObject -Namespace root/wmi -Class WmiMonitorBrightness); "
                                                     "$c = (Get-WmiObject -Namespace root/wmi -Class WmiMonitorBrightnessMethods); "
                                                     "$level = $b.CurrentBrightness - 10; "
                                                     "if ($level -lt 0) { $level = 0 }; "
                                                     "$c.WmiSetBrightness(1, $level)");
}

void HotkeyHandler:: brightnessToggle()
{
    qDebug() << "brightnessToggle called";
    QProcess::execute("powershell", QStringList() << "-Command"
                                                  << "$b = (Get-WmiObject -Namespace root/wmi -Class WmiMonitorBrightness); "
                                                     "$c = (Get-WmiObject -Namespace root/wmi -Class WmiMonitorBrightnessMethods); "
                                                     "if ($b.CurrentBrightness -gt 10) { $c.WmiSetBrightness(1, 0) } else { $c.WmiSetBrightness(1, 70) }");
}


#define WIN_KEY VK_LWIN
#define TAB_KEY VK_TAB
#define ENTER_KEY VK_RETURN
#define LEFT_ARROW VK_LEFT
#define RIGHT_ARROW VK_RIGHT

bool HotkeyHandler:: appSwitcherActive = false;

// Sends a single keypress (down + up)
void HotkeyHandler:: sendSingleKey(WORD key) {
    INPUT inputs[2] = {};

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = key;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = key;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(2, inputs, sizeof(INPUT));
}

// Sends a key combo (modifier + key), e.g. Win + Tab
void HotkeyHandler:: sendKeyCombo(WORD modifier, WORD key) {
    INPUT inputs[4] = {};

    // Press modifier
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = modifier;

    // Press key
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = key;

    // Release key
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = key;
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

    // Release modifier
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = modifier;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(4, inputs, sizeof(INPUT));
}

// Press encoder → open Task View or confirm selection
void HotkeyHandler:: activateAppSwitcher() {
    if (!appSwitcherActive) {
        qDebug() << "activateAppSwitcher: opening Win+Tab";
        sendKeyCombo(WIN_KEY, TAB_KEY);
        appSwitcherActive = true;
    } else {
        qDebug() << "activateAppSwitcher: selecting with Enter";
        sendSingleKey(ENTER_KEY);
        appSwitcherActive = false;
    }
}

// Rotate encoder right → move right in Task View
void HotkeyHandler::switchAppRight() {
    if (appSwitcherActive) {
        qDebug() << "switchAppRight: moving right";
        sendSingleKey(RIGHT_ARROW);
    }
}

// Rotate encoder left → move left in Task View
void HotkeyHandler::switchAppLeft() {
    if (appSwitcherActive) {
        qDebug() << "switchAppLeft: moving left";
        sendSingleKey(LEFT_ARROW);
    }
}


// Zoom in (Ctrl + Numpad '+')
void HotkeyHandler::zoomIn() {
    qDebug() << "zoomIn called";
    sendKeyCombo(VK_CONTROL, VK_ADD);
}

// Zoom out (Ctrl + Numpad '-')
void HotkeyHandler::zoomOut() {
    qDebug() << "zoomOut called";
    sendKeyCombo(VK_CONTROL, VK_SUBTRACT);
}

// Reset zoom to default (Ctrl + '0')
void HotkeyHandler::zoomReset() {
    qDebug() << "zoomReset called";
    sendKeyCombo(VK_CONTROL, '0');
}

// Move to the next tab (Ctrl + Page Down)
void HotkeyHandler::nextTab() {
    qDebug() << "nextTab called";
    sendKeyCombo(VK_CONTROL, VK_NEXT);
}

// Move to the previous tab (Ctrl + Page Up)
void HotkeyHandler::previousTab() {
    qDebug() << "previousTab called";
    sendKeyCombo(VK_CONTROL, VK_PRIOR);
}

#endif
