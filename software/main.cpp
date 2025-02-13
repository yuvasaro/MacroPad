#include "mainwindow.h"
#include <QApplication>
#include <vector>
#include <windows.h>

std::unordered_map<UINT, std::function<void()>> MainWindow::hotkeyActions;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    // Example 1: Assign F6 to open Notepad
    w.RegisterHotkey(VK_F6, []() {
        ShellExecuteW(NULL, L"open", L"notepad", NULL, NULL, SW_SHOWNORMAL);
    });

    // Example 2: Assign F7 to simulate Alt + Space
    std::vector<INPUT> inputs(4);
    inputs.resize(4);

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_MENU;  // Press ALT

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_SPACE;  // Press SPACE

    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = VK_SPACE;
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;  // Release SPACE

    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_MENU;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;  // Release ALT

    w.RegisterHotkey(VK_F7, [inputs]() mutable {
        SendInput(inputs.size(), inputs.data(), sizeof(INPUT));
    });



    w.show();

    return a.exec();
}
