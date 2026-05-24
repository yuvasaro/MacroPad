// mainwindow.cpp (refactored)
#include "mainwindow.h"
#include "profile.h"
#include "hotkeyhandler.h"
#include "imagecache.h"
#include "knobhandler.h"
#include "serialhandler.h"
#include "apptracker.h"
#include "iconextractor.h"
#include "keystrokerecorder.h"

#include <QApplication>
#include <QQmlEngine>
#include <QIcon>
#include <QAction>
#include <QVBoxLayout>
#include <QMenu>
#include <QQuickItem>
#include <QQuickWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QStringList>
#include <iostream>


#ifdef __APPLE__
    int KnobHandler::macVolume = KnobHandler::getSystemVolume();
#endif


MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    trayIcon(new QSystemTrayIcon(this)),
    trayMenu(new QMenu(this)),
    m_serialHandler(new SerialHandler(this)){

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
    qmlWidget->engine()->rootContext()->setContextProperty("keystrokeRecorder", this);


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

#ifdef Q_OS_MAC
namespace {
QString keyCodeToDisplayName(CGKeyCode code) {
    switch (code) {
    case kVK_Command:
    case kVK_RightCommand:
        return "Cmd";
    case kVK_Shift:
    case kVK_RightShift:
        return "Shift";
    case kVK_Control:
    case kVK_RightControl:
        return "Ctrl";
    case kVK_Option:
    case kVK_RightOption:
        return "Option";
    case kVK_Function:
        return "Fn";
    case kVK_Tab:
        return "Tab";
    case kVK_CapsLock:
        return "Caps Lock";
    case kVK_Delete:
        return "Delete";
    case kVK_Return:
        return "Return";
    case kVK_Space:
        return "Space";
    case kVK_Escape:
        return "Esc";
    case kVK_ANSI_A: return "A";
    case kVK_ANSI_B: return "B";
    case kVK_ANSI_C: return "C";
    case kVK_ANSI_D: return "D";
    case kVK_ANSI_E: return "E";
    case kVK_ANSI_F: return "F";
    case kVK_ANSI_G: return "G";
    case kVK_ANSI_H: return "H";
    case kVK_ANSI_I: return "I";
    case kVK_ANSI_J: return "J";
    case kVK_ANSI_K: return "K";
    case kVK_ANSI_L: return "L";
    case kVK_ANSI_M: return "M";
    case kVK_ANSI_N: return "N";
    case kVK_ANSI_O: return "O";
    case kVK_ANSI_P: return "P";
    case kVK_ANSI_Q: return "Q";
    case kVK_ANSI_R: return "R";
    case kVK_ANSI_S: return "S";
    case kVK_ANSI_T: return "T";
    case kVK_ANSI_U: return "U";
    case kVK_ANSI_V: return "V";
    case kVK_ANSI_W: return "W";
    case kVK_ANSI_X: return "X";
    case kVK_ANSI_Y: return "Y";
    case kVK_ANSI_Z: return "Z";
    case kVK_ANSI_0: return "0";
    case kVK_ANSI_1: return "1";
    case kVK_ANSI_2: return "2";
    case kVK_ANSI_3: return "3";
    case kVK_ANSI_4: return "4";
    case kVK_ANSI_5: return "5";
    case kVK_ANSI_6: return "6";
    case kVK_ANSI_7: return "7";
    case kVK_ANSI_8: return "8";
    case kVK_ANSI_9: return "9";
    case kVK_F1: return "F1";
    case kVK_F2: return "F2";
    case kVK_F3: return "F3";
    case kVK_F4: return "F4";
    case kVK_F5: return "F5";
    case kVK_F6: return "F6";
    case kVK_F7: return "F7";
    case kVK_F8: return "F8";
    case kVK_F9: return "F9";
    case kVK_F10: return "F10";
    case kVK_F11: return "F11";
    case kVK_F12: return "F12";
    default:
        return "";
    }
}

QStringList uniqueKeyNames(const std::vector<KeyCode>& keycodes) {
    QStringList keys;
    for (KeyCode code : keycodes) {
        const QString key = keyCodeToDisplayName(code);
        if (!key.isEmpty() && !keys.contains(key)) {
            keys.append(key);
        }
    }
    return keys;
}
}
#endif

bool MainWindow::startRecording() {
    if (KeystrokeRecorder::StartRecording()) {
        std::cout << "Recording started!" << std::endl;
        return true;
    }

    std::cerr << "Failed to start recording!" << std::endl;
    return false;
}

QString MainWindow::stopRecording() {
    std::vector<KeyCode> keycodes = KeystrokeRecorder::StopRecording();

    std::cout << "\n=== Recorded " << keycodes.size() << " keycodes ===" << std::endl;
    for (auto code : keycodes)
        std::cout << (int)code << " ";
    std::cout << "\n====================================" << std::endl;

#ifdef Q_OS_MAC
    return uniqueKeyNames(keycodes).join("+");
#else
    return QString::fromStdString(KeystrokeRecorder::ToString(keycodes));
#endif
}

bool MainWindow::isRecording() const {
    return KeystrokeRecorder::IsRecording();
}

QString MainWindow::browseExecutableFile() {
#ifdef Q_OS_MAC
    const QString startDirectory = "/Applications";
#else
    const QString startDirectory = QDir::homePath();
#endif

    return QFileDialog::getOpenFileName(
        this,
        "Select an Executable File",
        startDirectory,
        "Executable Files (*.exe *.app *.sh);;All Files (*)"
    );
}

QString MainWindow::browseImageFile() {
    return QFileDialog::getOpenFileName(
        this,
        "Select Key Image",
        QDir::homePath(),
        "Image Files (*.png *.jpg *.jpeg);;All Files (*)"
    );

}

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
    qInfo() << "Routing serial command:" << number
            << "active profile:"
            << (HotkeyHandler::currentProfile ? HotkeyHandler::currentProfile->getName() : QString("<none>"));

    /*We currently only have 1 knob:
     * Rotate Left: 71
     * Rotate Right: 72
     * Press Down: 73
    */
    if (number > 70 && number < 80)
    {
        qInfo() << "Command classified as encoder 1 input:" << number;
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
        qInfo() << "Command classified as encoder 2 input:" << number;
        if (number == 82)
            //KnobHandler::volumeUp();
            //KnobHandler::scrollUp();
            //KnobHandler::brightnessUp();
            //KnobHandler::switchAppLeft();
            //KnobHandler::zoomIn();
            //KnobHandler::nextTab();
            HotkeyHandler::executeEncoder(-1, HotkeyHandler::currentProfile,1);
        else if (number == 81)
            //KnobHandler::volumeDown();
            //KnobHandler::scrollDown();
            //KnobHandler::brightnessDown();
            //KnobHandler::switchAppRight();
            //KnobHandler::zoomOut();
            //KnobHandler::previousTab();
            HotkeyHandler::executeEncoder(-1, HotkeyHandler::currentProfile,2);
        else if (number == 83)
            //KnobHandler::toggleMute();
            //KnobHandler::autoScrollToggle();
            //KnobHandler::activateAppSwitcher();
            //KnobHandler::zoomReset();
            HotkeyHandler::executeEncoder(-1, HotkeyHandler::currentProfile,3);
        return;
    }

    //MacroKey triggering
    if (number>0 && number<10) {
        qInfo() << "Command classified as macro key:" << number;
        HotkeyHandler::executeHotkey(number, HotkeyHandler::currentProfile);
        return;
    }

    qWarning() << "Ignoring unrecognized serial command:" << number;
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
