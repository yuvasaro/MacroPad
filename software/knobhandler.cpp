#include "knobhandler.h"
#include "hotkeyhandler.h"
#include <qdebug.h>
#include <qlogging.h>
#include <QProcess>

#ifdef _WIN32
#include <windows.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <psapi.h>
#include <comdef.h>
#endif

bool KnobHandler::appSwitcherActive = false;

void KnobHandler:: scrollUp()
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

void KnobHandler:: scrollDown()
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

void KnobHandler::autoScrollToggle() {
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

#ifdef __APPLE__
    //TODO: Mac implementation
#endif
}

// ===== MAC HELPER FUNCTIONS =====
#ifdef __APPLE__
int KnobHandler::getSystemVolume() {
    FILE* pipe = popen("osascript -e 'output volume of (get volume settings)'", "r");
    if (!pipe) return -1;

    char buffer[128];
    if (fgets(buffer, sizeof(buffer), pipe) == nullptr) {
        pclose(pipe);
        return -1;
    }
    pclose(pipe);

    try {
        return std::stof(buffer);
    } catch (...) {
        return -1;
    }
}

bool KnobHandler::setSystemVolume(int volume) {
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;
    std::string command = "osascript -e 'set volume output volume " + std::to_string(volume) + "'";
    qDebug() << "Set system volume to" << volume;
    return (system(command.c_str()) == 0);
}
// ================================
#endif

// Increase volume
void KnobHandler::volumeUp() {
#ifdef _WIN32
    // TODO: Windows implementation
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
    KnobHandler::macVolume = (KnobHandler::macVolume >= 100) ? KnobHandler::macVolume : KnobHandler::macVolume + 6;
    setSystemVolume(KnobHandler::macVolume);
#endif
}

// Increase volume
void KnobHandler::volumeDown() {
#ifdef _WIN32
    // TODO: Windows implementation
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
    KnobHandler::macVolume = (KnobHandler::macVolume <= 0) ? KnobHandler::macVolume : KnobHandler::macVolume - 6;
    setSystemVolume(KnobHandler::macVolume);
#endif
}

// Increase volume
void KnobHandler::toggleMute() {
#ifdef _WIN32
    // TODO: Windows implementation
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
    FILE* pipe = popen("osascript -e 'output muted of (get volume settings)'", "r");
    if (!pipe) return;

    char buffer[16];
    fgets(buffer, sizeof(buffer), pipe);
    pclose(pipe);

    bool isMuted = std::string(buffer).find("true") != std::string::npos;

    if (isMuted)
        std::system("osascript -e \"set volume without output muted\"");
    else
        std::system("osascript -e \"set volume with output muted\"");
#endif
}

// Increase screen brightness by 10%
void KnobHandler::brightnessUp()
{
#ifdef   _WIN32
    qDebug() << "brightnessUp called";
    QProcess::execute("powershell", QStringList() << "-Command"
                                                  << "$b = (Get-WmiObject -Namespace root/wmi -Class WmiMonitorBrightness); "
                                                     "$c = (Get-WmiObject -Namespace root/wmi -Class WmiMonitorBrightnessMethods); "
                                                     "$level = $b.CurrentBrightness + 10; "
                                                     "if ($level -gt 100) { $level = 100 }; "
                                                     "$c.WmiSetBrightness(1, $level)");
#endif

#ifdef __APPLE__
    //TODO: Mac implementation
#endif
}

void KnobHandler::brightnessDown()
{
#ifdef   _WIN32
    qDebug() << "brightnessDown called";
    QProcess::execute("powershell", QStringList() << "-Command"
                                                  << "$b = (Get-WmiObject -Namespace root/wmi -Class WmiMonitorBrightness); "
                                                     "$c = (Get-WmiObject -Namespace root/wmi -Class WmiMonitorBrightnessMethods); "
                                                     "$level = $b.CurrentBrightness - 10; "
                                                     "if ($level -lt 0) { $level = 0 }; "
                                                     "$c.WmiSetBrightness(1, $level)");
#endif

#ifdef __APPLE__
    //TODO: Mac implementation
#endif
}

void KnobHandler:: brightnessToggle()
{
#ifdef  _WIN32
    qDebug() << "brightnessToggle called";
    QProcess::execute("powershell", QStringList() << "-Command"
                                                  << "$b = (Get-WmiObject -Namespace root/wmi -Class WmiMonitorBrightness); "
                                                     "$c = (Get-WmiObject -Namespace root/wmi -Class WmiMonitorBrightnessMethods); "
                                                     "if ($b.CurrentBrightness -gt 10) { $c.WmiSetBrightness(1, 0) } else { $c.WmiSetBrightness(1, 70) }");
#endif

#ifdef __APPLE__
    //TODO: Mac implementation
#endif
}

#ifdef _WIN32
#define WIN_KEY VK_LWIN
#define TAB_KEY VK_TAB
#define ENTER_KEY VK_RETURN
#define LEFT_ARROW VK_LEFT
#define RIGHT_ARROW VK_RIGHT
#endif

#ifdef _WIN32
// Sends a single keypress (down + up)
void KnobHandler::sendSingleKey(WORD key) {
    INPUT inputs[2] = {};

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = key;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = key;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(2, inputs, sizeof(INPUT));
}
#endif

#ifdef _WIN32
// Sends a key combo (modifier + key), e.g. Win + Tab
void KnobHandler::sendKeyCombo(WORD modifier, WORD key) {
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
#endif

// Press encoder → open Task View or confirm selection
void KnobHandler::activateAppSwitcher() {
#ifdef _WIN32
    if (!appSwitcherActive) {
        qDebug() << "activateAppSwitcher: opening Win+Tab";
        sendKeyCombo(WIN_KEY, TAB_KEY);
        appSwitcherActive = true;
    } else {
        qDebug() << "activateAppSwitcher: selecting with Enter";
        sendSingleKey(ENTER_KEY);
        appSwitcherActive = false;
    }
#endif

#ifdef __APPLE__
    if (!appSwitcherActive) {
        qDebug() << "activateAppSwitcher: opening Cmd+Tab";
        QStringList keys = {"cmd", "tab"};
        HotkeyHandler::pressAndReleaseKeys(keys);
        appSwitcherActive = true;
    } else {
        qDebug() << "activateAppSwitcher: selecting with Return";
        QStringList keys = {"return"};
        HotkeyHandler::pressAndReleaseKeys(keys);
        appSwitcherActive = false;
    }
#endif
}

// Rotate encoder right → move right in Task View
void KnobHandler::switchAppRight() {
#ifdef _WIN32
    if (appSwitcherActive) {
        qDebug() << "switchAppRight: moving right";
        sendSingleKey(RIGHT_ARROW);
    }
#endif

#ifdef __APPLE__
    //TODO: Mac implementation
    if (appSwitcherActive) {
        QStringList keys = {"cmd", "tab"};
        HotkeyHandler::pressAndReleaseKeys(keys);
    }
#endif
}

// Rotate encoder left → move left in Task View
void KnobHandler::switchAppLeft() {
#ifdef _WIN32
    if (appSwitcherActive) {
        qDebug() << "switchAppLeft: moving left";
        sendSingleKey(LEFT_ARROW);
    }
#endif

#ifdef __APPLE__
    //TODO: Mac implementation
    if (appSwitcherActive) {
        QStringList keys = {"cmd", "shift", "tab"};
        HotkeyHandler::pressAndReleaseKeys(keys);
    }
#endif
}


// Zoom in (Ctrl + Numpad '+')
void KnobHandler::zoomIn() {
#ifdef _WIN32
    qDebug() << "zoomIn called";
    sendKeyCombo(VK_CONTROL, VK_ADD);
#endif

#ifdef __APPLE__
    //TODO: Mac implementation
    QStringList keys = {"cmd", "="};
    HotkeyHandler::pressAndReleaseKeys(keys);
#endif
}

// Zoom out (Ctrl + Numpad '-')
void KnobHandler::zoomOut() {
#ifdef _WIN32
    qDebug() << "zoomOut called";
    sendKeyCombo(VK_CONTROL, VK_SUBTRACT);
#endif

#ifdef __APPLE__
    //TODO: Mac implementation
    QStringList keys = {"cmd", "shift", "-"};
    HotkeyHandler::pressAndReleaseKeys(keys);
#endif
}

// Reset zoom to default (Ctrl + '0')
void KnobHandler::zoomReset() {
#ifdef _WIN32
    qDebug() << "zoomReset called";
    sendKeyCombo(VK_CONTROL, '0');
#endif

#ifdef __APPLE__
    //TODO: Mac implementation
    QStringList keys = {"cmd", "0"};
    HotkeyHandler::pressAndReleaseKeys(keys);
#endif
}

// Move to the next tab (Ctrl + Page Down)
void KnobHandler::nextTab() {
#ifdef _WIN32
    qDebug() << "nextTab called";
    sendKeyCombo(VK_CONTROL, VK_NEXT);
#endif

#ifdef __APPLE__
    //TODO: Mac implementation
#endif
}

// Move to the previous tab (Ctrl + Page Up)
void KnobHandler::previousTab() {
#ifdef _WIN32
    qDebug() << "previousTab called";
    sendKeyCombo(VK_CONTROL, VK_PRIOR);
#endif

#ifdef __APPLE__
    //TODO: Mac implementation
#endif
}

#ifdef _WIN32
void adjustAppVolume(QString name, float volumeStep) {
    const wchar_t* targetApp = reinterpret_cast<const wchar_t *>(name.utf16());
    CoInitialize(nullptr);

    IMMDeviceEnumerator* pEnumerator = nullptr;
    IMMDevice* pDevice = nullptr;
    IAudioSessionManager2* pSessionManager = nullptr;
    IAudioSessionEnumerator* pSessionEnumerator = nullptr;

    if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&pEnumerator)))) return;
    if (FAILED(pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice))) return;
    if (FAILED(pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, nullptr, (void**)&pSessionManager))) return;
    if (FAILED(pSessionManager->GetSessionEnumerator(&pSessionEnumerator))) return;

    int sessionCount = 0;
    pSessionEnumerator->GetCount(&sessionCount);

    for (int i = 0; i < sessionCount; ++i) {
        IAudioSessionControl* pSessionControl = nullptr;
        pSessionEnumerator->GetSession(i, &pSessionControl);

        IAudioSessionControl2* pSessionControl2 = nullptr;
        if (SUCCEEDED(pSessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&pSessionControl2))) {
            DWORD processId;
            pSessionControl2->GetProcessId(&processId);

            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
            if (hProcess) {
                WCHAR processName[MAX_PATH];
                if (GetModuleBaseNameW(hProcess, nullptr, processName, MAX_PATH) > 0) {
                    if (_wcsicmp(processName, targetApp) == 0) {
                        ISimpleAudioVolume* pVolume = nullptr;
                        if (SUCCEEDED(pSessionControl2->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&pVolume))) {
                            float currentVolume = 0.0f;
                            pVolume->GetMasterVolume(&currentVolume);

                            float newVolume = currentVolume + volumeStep;
                            if (newVolume < 0.0f) newVolume = 0.0f;
                            if (newVolume > 1.0f) newVolume = 1.0f;

                            pVolume->SetMasterVolume(newVolume, nullptr);
                            pVolume->Release();
                        }
                    }
                }
                CloseHandle(hProcess);
            }
            pSessionControl2->Release();
        }
        pSessionControl->Release();
    }

    pSessionEnumerator->Release();
    pSessionManager->Release();
    pDevice->Release();
    pEnumerator->Release();

    CoUninitialize();
}
#endif


void KnobHandler::appVolumeUp(){
    adjustAppVolume("a", .01);
}
void KnobHandler::appVolumeDown(){
    adjustAppVolume("a", -.01);
}
