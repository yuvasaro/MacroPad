#include <QApplication>
#include "mainwindow.h"
#include "apptracker.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow mainWindow;
    mainWindow.show();

    AppTracker appTracker;

    return a.exec();
}
