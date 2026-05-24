#include <QApplication>
#include <QQuickStyle>
#include <QIcon>
#include "mainwindow.h"


int main(int argc, char *argv[])
{
    QQuickStyle::setStyle("Basic");

    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/Macropad_icon.ico"));

    MainWindow mainWindow;
    mainWindow.show();

    return a.exec();
}
