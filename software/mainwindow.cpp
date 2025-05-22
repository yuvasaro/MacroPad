// mainwindow.cpp (refactored)
#include "mainwindow.h"
#include "fileio.h"
#include "profile.h"
#include "hotkeyhandler.h"
#include "knobhandler.h"
#include "serialhandler.h"
#include "apptracker.h"

#include <QApplication>
#include <QQmlEngine>
#include <QIcon>
#include <QAction>
#include <QVBoxLayout>
#include <QMenu>
#include <QQuickItem>
#include <QQuickWidget>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>


MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    trayIcon(new QSystemTrayIcon(this)),
    trayMenu(new QMenu(this)),
    m_serialHandler(new SerialHandler(this)) {

    setWindowTitle("MacroPad - Configuration");

    qmlWidget = new QQuickWidget(this);
    qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    FileIO *fileIO = new FileIO(this);
    Macro *macro = new Macro(this);

    qmlRegisterType<FileIO>("FileIO", 1, 0, "FileIO");
    qmlRegisterType<Macro>("Macro", 1, 0, "Macro");


    hotkeyHandler = new HotkeyHandler(this);
    hotkeyHandler->initializeProfiles();

    // Register with QML
    qmlWidget->engine()->rootContext()->setContextProperty("fileIO", fileIO);
    qmlWidget->engine()->rootContext()->setContextProperty("Macro", macro);
    //qmlWidget->engine()->rootContext()->setContextProperty("profileInstance", HotkeyHandler::profileManager);
    qmlWidget->engine()->rootContext()->setContextProperty("currentProfile", HotkeyHandler::currentProfile);
    qmlWidget->engine()->rootContext()->setContextProperty("hotkeyHandler", hotkeyHandler);
    qmlWidget->engine()->rootContext()->setContextProperty("mainWindow", this);
    qmlWidget->setSource(QUrl("qrc:/Main.qml"));


    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    layout->addWidget(qmlWidget);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    createTrayIcon();

    qDebug() << "[MainWindow] &appTracker =" << &appTracker
             << " QObject parent? =" << appTracker.parent()
             << " hotkeyHandler =" << hotkeyHandler;

    auto ok = QObject::connect(&appTracker, &AppTracker::appChanged, hotkeyHandler, &HotkeyHandler::switchCurrentProfile,Qt::DirectConnection);

    QObject::connect(
        &appTracker,
        &AppTracker::appChanged,
        this,  // or nullptr
        [](const QString &appName){
            qDebug() << "[Debug] appChanged signal EMITTED for:" << appName;
        },
        Qt::DirectConnection  // fire immediately, regardless of threads
        );

    //is connecting
    qDebug() << "[Connect] success?" << bool(ok);

    connect(m_serialHandler, &SerialHandler::dataReceived, this, &MainWindow::onDataReceived);
}

MainWindow::~MainWindow() {}

void MainWindow::createTrayIcon() {
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::warning(this, "Warning", "System tray is not available!");
        return;
    }

#ifdef Q_OS_MAC
    QString iconPath = QCoreApplication::applicationDirPath() + "/../Resources/MPIcon.png";
    QIcon icon(iconPath);
    trayIcon->setIcon(icon);
#endif

#ifdef _WIN32
    QString iconPath = QCoreApplication::applicationDirPath() + "/../../MPIcon.ico";
    QIcon icon(iconPath);
    trayIcon->setIcon(icon);
    this->setWindowIcon(icon);
#endif

    trayIcon->setToolTip("Configuration Software Running");

    QAction *restoreAction = new QAction("Show Window", this);
    QAction *exitAction = new QAction("Exit", this);

    connect(restoreAction, &QAction::triggered, this, &MainWindow::showWindow);
    connect(exitAction, &QAction::triggered, this, &MainWindow::exitApplication);

    trayMenu->addAction(restoreAction);
    trayMenu->addAction(exitAction);

    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();
}

void MainWindow::callHotkeyHandler(Profile* profile, int keyNum, const QString& type, const QString& content) {
    HotkeyHandler::registerGlobalHotkey(profile, keyNum, type, content);
}

void MainWindow::onDataReceived(int number)
{
    //Volume Knob
    if (number > 70 && number < 80)
    {
        if (number == 72){
            HotkeyHandler::executeEncoder(-2, HotkeyHandler::currentProfile,1);
        qDebug("executeEncoder");
            //KnobHandler::volumeUp();
            //KnobHandler::scrollUp();
            //KnobHandler::brightnessUp();
            //KnobHandler::switchAppLeft();
            //KnobHandler::zoomIn();
            //KnobHandler::nextTab();
        }
        else if (number == 71)
            HotkeyHandler::executeEncoder(-2, HotkeyHandler::currentProfile,1);
            //KnobHandler::volumeDown();
            //KnobHandler::scrollDown();
            //KnobHandler::brightnessDown();
            //KnobHandler::switchAppRight();
            //KnobHandler::zoomOut();
            //KnobHandler::previousTab();
        else if (number == 73)
            HotkeyHandler::executeEncoder(-2, HotkeyHandler::currentProfile,1);
            //KnobHandler::mute();
            //KnobHandler::autoScrollToggle();
            //KnobHandler::activateAppSwitcher();
            //KnobHandler::zoomReset();
        return;
    }

    //MacroKey triggering
    if (number>0 && number<10) {
        HotkeyHandler::executeHotkey(number, HotkeyHandler::currentProfile);
    }
}
void MainWindow::closeEvent(QCloseEvent *event) {
    if (trayIcon->isVisible()) {
        hide();
        event->ignore();
        toggleDockIcon(false);
    }
}

void MainWindow::showWindow() {
    toggleDockIcon(true);
    showNormal();
    activateWindow();
}

void MainWindow::exitApplication() {
    trayIcon->hide();
    QApplication::quit();
}

void MainWindow::toggleDockIcon(bool show) {
#ifdef Q_OS_MAC
    ProcessSerialNumber psn = { 0, kCurrentProcess };
    if (show) {
        TransformProcessType(&psn, kProcessTransformToForegroundApplication);
    } else {
        TransformProcessType(&psn, kProcessTransformToUIElementApplication);
    }
#endif
}

