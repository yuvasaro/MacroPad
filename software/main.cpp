#include "mainwindow.h"
#include "macro.h"
#include <QApplication>
#include <iostream>


int main(int argc, char *argv[])
{
    std::vector<WORD> result = Macro::translateKeys("H+CTRL+C+T+V");
    for (WORD key : result) {
        qDebug() << key << " ";
    }
    qDebug() << "hello";
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
