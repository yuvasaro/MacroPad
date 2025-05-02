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

    void MainWindow::executeHotkey(int hotKeyNum, Profile* profileInstance) {
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

    profile->setMacro(keyNum, type, content);
#elif __APPLE__
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

static void volumeUp()
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

static void volumeDown()
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

static void mute()
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


// Scroll functions

static void scrollUp()
{
#ifdef _WIN32
    // qDebug() << "scrollUp called on Windows";
    // QScrollArea* scrollArea = ui->scrollArea;
    // QScrollBar* vScrollBar = scrollArea->verticalScrollBar();
    // if (vScrollBar) {
    //     vScrollBar->setValue(vScrollBar->value() - 50);
    // }
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

static void scrollDown()
{
#ifdef _WIN32
    // qDebug() << "scrollDown called on Windows";
    // QScrollArea* scrollArea = ui->scrollArea;
    // QScrollBar* vScrollBar = scrollArea->verticalScrollBar();
    // if (vScrollBar) {
    //     vScrollBar->setValue(vScrollBar->value() + 50);
    // }
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
