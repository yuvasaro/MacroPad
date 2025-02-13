#include "mainwindow.h"
#include <QApplication>
#include <windows.h>

std::unordered_map<UINT, std::function<void()>> MainWindow::hotkeyActions;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
