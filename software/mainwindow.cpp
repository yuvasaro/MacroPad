// mainwindow.cpp (refactored)
#include "mainwindow.h"
#include "profile.h"
#include "hotkeyhandler.h"
#include "imagecache.h"
#include "knobhandler.h"
#include "serialhandler.h"
#include "apptracker.h"
#include "iconextractor.h"

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


#ifdef __APPLE__
    int KnobHandler::macVolume = KnobHandler::getSystemVolume();
#endif


MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    trayIcon(new QSystemTrayIcon(this)),
    trayMenu(new QMenu(this)),
    m_serialHandler(new SerialHandler(this)) {

    setWindowTitle("MacroPad - Configuration");

    qmlWidget = new QQuickWidget(this);
    qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    Macro *macro = new Macro(this);
    IconExtractor* iconExtractor = new IconExtractor(this);

    qmlRegisterType<Macro>("Macro", 1, 0, "Macro");
    qmlRegisterType<IconExtractor>("IconExtractor", 1, 0, "IconExtractor");
    qmlRegisterSingletonType<ImageCache>("ImageCache", 1, 0, "ImageCache", [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject* {
        return ImageCache::instance();
    });


    //Initialize hotkeyHandler and profile
    hotkeyHandler = new HotkeyHandler(this);
    hotkeyHandler->initializeProfiles();

    //serialHandler = new SerialHandler(this);
    hotkeyHandler->setSerialHandler(m_serialHandler);

    // Register with QML
    qmlWidget->engine()->rootContext()->setContextProperty("Macro", macro);
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

    // Connecting apptracker signals to switchProfile.
    QObject::connect(&appTracker, &AppTracker::appChanged, hotkeyHandler, &HotkeyHandler::switchCurrentProfile,Qt::DirectConnection);
    // Connecting MacroPad with SerialHandler.
    connect(m_serialHandler, &SerialHandler::dataReceived, this, &MainWindow::onDataReceived);
}

MainWindow::~MainWindow() {}

/*
 * Loading the Icon for the app.
 */
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

/*
 * Nesting registerGlobalHotkey() into a MainWindow function, because qml can't get it from hotkeyhandler.
 */
void MainWindow::callHotkeyHandler(Profile* profile, int keyNum, const QString& type, const QString& content) {
    HotkeyHandler::registerGlobalHotkey(profile, keyNum, type, content);
}

/*
 * Function that connects with Macropad
 */
void MainWindow::onDataReceived(int number)
{
    /*We currently only have 1 knob:
     * Rotate Left: 71
     * Rotate Right: 72
     * Press Down: 73
    */
    if (number > 70 && number < 80)
    {
        //Manually Set to the Encoder 1 (macro key -2)
        if (number == 72)
            HotkeyHandler::executeEncoder(-2, HotkeyHandler::currentProfile,1);
        else if (number == 71)
            HotkeyHandler::executeEncoder(-2, HotkeyHandler::currentProfile,2);
        else if (number == 73)
            HotkeyHandler::executeEncoder(-2, HotkeyHandler::currentProfile,3);
            //KnobHandler::mute();
            //KnobHandler::autoScrollToggle();
        return;
    }

    if (number > 80 && number < 90)
    {
        if (number == 82)
            //KnobHandler::volumeUp();
            //KnobHandler::scrollUp();
            //KnobHandler::brightnessUp();
            //KnobHandler::switchAppLeft();
            //KnobHandler::zoomIn();
            KnobHandler::nextTab();
        else if (number == 81)
            //KnobHandler::volumeDown();
            //KnobHandler::scrollDown();
            //KnobHandler::brightnessDown();
            //KnobHandler::switchAppRight();
            //KnobHandler::zoomOut();
            KnobHandler::previousTab();
        else if (number == 83)
            KnobHandler::toggleMute();
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

// ----------------------------------------------------------------------------------------

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
