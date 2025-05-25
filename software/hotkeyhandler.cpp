// hotkeyhandler.cpp
#include "hotkeyhandler.h"
#include "profile.h"
#include "apptracker.h"
#include <QDir>
#include <QDebug>
#include <QMap>
#include <iostream>
#include <thread>
#include <chrono>
#include <QProcess>
#include <QFileInfo>
#include <QFileInfoList>
#include "knobhandler.h"

#define DEBUG

// profileManager in this file refers to the profile that is selected from the dropdown in the UI
Profile* HotkeyHandler::profileManager;

// currentProfile refers to the profile that matches the name of the application the user is on
Profile* HotkeyHandler::currentProfile;
QList<Profile*> profiles;

HotkeyHandler::HotkeyHandler(QObject* parent)
    : QObject(parent){}

HotkeyHandler::~HotkeyHandler(){
    qDeleteAll(profiles);
    profiles.clear();
}

// ------- the following functions are required to expose the profiles list to QML -----

// required profileCount function for QML_PROPERTY
qsizetype HotkeyHandler::profileCount(QQmlListProperty<Profile> *list) {
    auto profiles = static_cast<QList<Profile*>*>(list->data);
    return profiles->size();
}

// required profileAt function for QML_PROPERTY
Profile *HotkeyHandler::profileAt(QQmlListProperty<Profile> *list, qsizetype index) {
    auto profiles = static_cast<QList<Profile*>*>(list->data);
    return profiles->at(index);
}

// getter for QML to access profiles
QQmlListProperty<Profile> HotkeyHandler::getProfiles() {
    return QQmlListProperty<Profile>(
        this,
        &profiles, // use MainWindow instance as the data object
        &HotkeyHandler::profileCount,
        &HotkeyHandler::profileAt
        );
}

// ----------------------------------------------------------------------------------------


/*
 * This is the function that will load all the profiles that are saved in the user's config directory
 * with the corresponding hotkeys and executables
 *
 * For first time users without preexisting profiles, new ones will be loaded and saved to config.
 * From there, every time they run the application, their saved profiles will be loaded
 */
void HotkeyHandler::initializeProfiles() {
    QString names[6] = {"General", "Profile 1", "Profile 2", "Profile 3", "Profile 4", "Profile 5"};
    QString apps[6] = {"", "", "", "", "", ""};

    for (int i = 0; i < 6; ++i) {

        Profile* profile = Profile::loadProfile(names[i]);

        // if the profile with the corresponding name (e.g. "Profile 1") does not exist, create it
        if (!profile) {
            profile = new Profile("","");
            profile->setName(names[i]);
            profile->setApp(apps[i]);
            profile->saveProfile();
        }

        // for all macros of the current profile in the loop, we have to register the hotkey
        for (int keyNum = 1; keyNum <= 9; ++keyNum) {
            QSharedPointer<Macro> macro = profile->getMacro(keyNum);
            if (!macro.isNull()) {
                registerGlobalHotkey(profile, keyNum, macro->getType(), macro->getContent());
            }
        }

        profiles.append(profile);
    }

    profileManager = profiles[0];
    currentProfile = profiles[0];
}


/*
 * This function encompasses our main profile-switching logic. In mainwindow.cpp, there is a line that connects the appTracker
 * to this function, so that every time the user switches their app focus, this function is called
 *
 * It essentially loops through the list of 6 profiles and sets the currentProfile object to whichever
 * profile matches the name of the app the user is focused on
 *
 */
void HotkeyHandler::switchCurrentProfile(const QString& appName) {
    qDebug() << "switchCurrentProfile now";
    qDebug() << "Current app:" << appName;
    // for (Profile* profile : profiles) {
    //     if (profile->getApp() == appName) {
    //         currentProfile = profile;
    //         qDebug() << "Current profile set to:" << currentProfile->getName();
    //         return;
    //     }
    // }
    bool found = false;
    for(int i=0;i<profiles.size();i++)
    {
        if (profiles[i]->getApp() == appName)
        {
            found = true;
            currentProfile = profiles[i];
            qDebug() << "Current profile set to:" << currentProfile->getName();
            if (serialHandler)
                serialHandler->sendProfile(i + 10);
            return;
        }
    }
    if(!found)
    {
        currentProfile = profiles[0];
        if (serialHandler)
            serialHandler->sendProfile(10);
        qDebug() << "No profile with this app. Set to General";
        return;
    }
    return;
}

void HotkeyHandler::setProfileManager(Profile* profile) {
    if (profileManager != profile) {
        profileManager = profile;
        emit profileManagerChanged();
    }
}

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

    qDebug() << "Executing macro from" << profileInstance->getName()<< hotKeyNum << type << content;

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
QMap<int, EventHotKeyRef> HotkeyHandler::registeredHotkeys;
static EventHandlerUPP eventHandlerUPP;
static const std::map<int, int> debugKeyMap = {
    {1, kVK_ANSI_1}, {2, kVK_ANSI_2}, {3, kVK_ANSI_3}, {4, kVK_ANSI_4}, {5, kVK_ANSI_5},
    {6, kVK_ANSI_6}, {7, kVK_ANSI_7}, {8, kVK_ANSI_8}, {9, kVK_ANSI_9}
};

const QMap<QString, CGKeyCode> keyMap = {
    // Modifier keys
    {"cmd", kVK_Command},
    {"shift", kVK_Shift},
    {"ctrl", kVK_Control},
    {"option", kVK_Option},
    {"fn", kVK_Function},

    // Letters
    {"a", kVK_ANSI_A}, {"b", kVK_ANSI_B}, {"c", kVK_ANSI_C}, {"d", kVK_ANSI_D},
    {"e", kVK_ANSI_E}, {"f", kVK_ANSI_F}, {"g", kVK_ANSI_G}, {"h", kVK_ANSI_H},
    {"i", kVK_ANSI_I}, {"j", kVK_ANSI_J}, {"k", kVK_ANSI_K}, {"l", kVK_ANSI_L},
    {"m", kVK_ANSI_M}, {"n", kVK_ANSI_N}, {"o", kVK_ANSI_O}, {"p", kVK_ANSI_P},
    {"q", kVK_ANSI_Q}, {"r", kVK_ANSI_R}, {"s", kVK_ANSI_S}, {"t", kVK_ANSI_T},
    {"u", kVK_ANSI_U}, {"v", kVK_ANSI_V}, {"w", kVK_ANSI_W}, {"x", kVK_ANSI_X},
    {"y", kVK_ANSI_Y}, {"z", kVK_ANSI_Z},

    // Numbers
    {"0", kVK_ANSI_0}, {"1", kVK_ANSI_1}, {"2", kVK_ANSI_2}, {"3", kVK_ANSI_3},
    {"4", kVK_ANSI_4}, {"5", kVK_ANSI_5}, {"6", kVK_ANSI_6}, {"7", kVK_ANSI_7},
    {"8", kVK_ANSI_8}, {"9", kVK_ANSI_9},

    // Symbols and punctuation
    {"`",  kVK_ANSI_Grave},
    {"-",  kVK_ANSI_Minus},
    {"=",  kVK_ANSI_Equal},
    {"[",  kVK_ANSI_LeftBracket},
    {"]",  kVK_ANSI_RightBracket},
    {"\\", kVK_ANSI_Backslash},
    {";",  kVK_ANSI_Semicolon},
    {"'",  kVK_ANSI_Quote},
    {",",  kVK_ANSI_Comma},
    {".",  kVK_ANSI_Period},
    {"/",  kVK_ANSI_Slash},

    // Whitespace and control
    {"space",  kVK_Space},
    {"tab",    kVK_Tab},
    {"return", kVK_Return},
    {"enter",  kVK_Return}, // Alias
    {"delete", kVK_Delete}, // Backspace
    {"forwarddelete", kVK_ForwardDelete},
    {"esc",    kVK_Escape},
    {"escape", kVK_Escape},

    // Arrow keys
    {"left",  kVK_LeftArrow},
    {"right", kVK_RightArrow},
    {"up",    kVK_UpArrow},
    {"down",  kVK_DownArrow},

    // Navigation
    {"home",    kVK_Home},
    {"end",     kVK_End},
    {"pageup",  kVK_PageUp},
    {"pagedown",kVK_PageDown},
    {"help",    kVK_Help},

    // Function keys
    {"f1",  kVK_F1},  {"f2",  kVK_F2},  {"f3",  kVK_F3},  {"f4",  kVK_F4},
    {"f5",  kVK_F5},  {"f6",  kVK_F6},  {"f7",  kVK_F7},  {"f8",  kVK_F8},
    {"f9",  kVK_F9},  {"f10", kVK_F10}, {"f11", kVK_F11}, {"f12", kVK_F12},
    {"f13", kVK_F13}, {"f14", kVK_F14}, {"f15", kVK_F15}, {"f16", kVK_F16},
    {"f17", kVK_F17}, {"f18", kVK_F18}, {"f19", kVK_F19}, {"f20", kVK_F20},
};

void HotkeyHandler::pressAndReleaseKeys(const QStringList& keys) {
    QList<CGEventRef> keysDown;
    QList<CGEventRef> keysUp;
    CGEventSourceRef eventSource = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
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
            CGEventRef keyDown = CGEventCreateKeyboardEvent(eventSource, keyMap[key], true);
            CGEventSetFlags(keyDown, flags); // Apply modifier flags
            keysDown.push_back(keyDown);

            // Create keyup event for normal key
            CGEventRef keyUp = CGEventCreateKeyboardEvent(eventSource, keyMap[key], false);
            CGEventSetFlags(keyUp, flags); // Apply modifier flags
            keysUp.push_back(keyUp);
        }
    }

    // Post keydown events
    for (CGEventRef keyDown : keysDown) {
        CGEventPost(kCGHIDEventTap, keyDown);
    }

    usleep(1000); // Sleep for 10ms to simulate key press duration

    // Post keyup events in reverse order (release main key first, then modifier)
    for (CGEventRef keyUp : keysUp) {
        CGEventPost(kCGHIDEventTap, keyUp);
    }

    // Release resources
    for (CGEventRef key : keysDown) {
        CFRelease(key);
    }
    for (CGEventRef key : keysUp) {
        CFRelease(key);
    }
    CFRelease(eventSource);
}

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

    executeHotkey(hotKeyID.id, currentProfile);

    return noErr;
}

void HotkeyHandler::executeHotkey(int hotKeyNum, Profile* profileInstance) {
    QSharedPointer<Macro> macro = profileInstance->getMacro(hotKeyNum);

    if (!macro.isNull()) {
        qDebug() << hotKeyNum << "key pressed! Type:" << macro->getType() << "Content:" << macro->getContent();

        const QString& type = macro->getType();
        const QString& content = macro->getContent();

        if (type == "keystroke") {
            QStringList keys = content.toLower().split("+");
            pressAndReleaseKeys(keys);
            //KnobHandler::brightnessUp();
        } else if (type == "executable") {
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

    // Register a runtime lookup lambda
    hotkeyActions[vkCode] = [keyNum]() {
        if (!HotkeyHandler::currentProfile) {
            OutputDebugStringW(L"No active profile.\n");
            return;
        }

        QSharedPointer<Macro> macro = HotkeyHandler::currentProfile->getMacro(keyNum);
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
    OSStatus status = RegisterEventHotKey(debugKeyMap.at(keyNum), 0, hotkeyID, GetApplicationEventTarget(), 0, &hotkeyRef);
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
