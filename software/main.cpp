#include <QApplication>
#include <QQuickStyle>
#include "mainwindow.h"


int main(int argc, char *argv[])
{
    QQuickStyle::setStyle("Basic");

    QApplication a(argc, argv);

    MainWindow mainWindow;
    mainWindow.show();

    return a.exec();
}
