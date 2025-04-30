// mainwindow.cpp (refactored)
#include "mainwindow.h"
#include "fileio.h"
#include "profile.h"
#include "hotkeyhandler.h"
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


QList<Profile*> profiles;


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


    initializeProfiles();

    // Register with QML
    qmlWidget->engine()->rootContext()->setContextProperty("fileIO", fileIO);
    qmlWidget->engine()->rootContext()->setContextProperty("Macro", macro);
    qmlWidget->engine()->rootContext()->setContextProperty("profileInstance", HotkeyHandler::profileManager);
    qmlWidget->engine()->rootContext()->setContextProperty("mainWindow", this);
    qmlWidget->setSource(QUrl("qrc:/Main.qml"));


    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    layout->addWidget(qmlWidget);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    createTrayIcon();


    connect(m_serialHandler, &SerialHandler::dataReceived,
            this, &MainWindow::onDataReceived);
    //app switch currently in progress
    QObject::connect(&appTracker, &AppTracker::appChanged, this, &MainWindow::switchCurrentProfile);

}

MainWindow::~MainWindow() {
    qDeleteAll(profiles);
    profiles.clear();
}

// required profileCount function for QML_PROPERTY
qsizetype MainWindow::profileCount(QQmlListProperty<Profile> *list) {
    auto profiles = static_cast<QList<Profile*>*>(list->data);
    return profiles->size();
}

// required profileAt function for QML_PROPERTY
Profile *MainWindow::profileAt(QQmlListProperty<Profile> *list, qsizetype index) {
    auto profiles = static_cast<QList<Profile*>*>(list->data);
    return profiles->at(index);
}

// getter for QML to access profiles
QQmlListProperty<Profile> MainWindow::getProfiles() {
    return QQmlListProperty<Profile>(
        this,
        &profiles, // use MainWindow instance as the data object
        &MainWindow::profileCount,
        &MainWindow::profileAt
        );
}

void MainWindow::setProfileInstance(Profile* profile) {
    if (profileInstance != profile) {
        profileInstance = profile;
        emit profileInstanceChanged();
    }
}

void MainWindow::initializeProfiles() {
    QString names[6] = {"General", "Profile 1", "Profile 2", "Profile 3", "Profile 4", "Profile 5"};
    QString apps[6] = {"", "Google Chrome", "Qt Creator", "MacroPad", "Discord", "Spotify"};

    for (int i = 0; i < 6; ++i) {

        Profile* profile = Profile::loadProfile(names[i]);

        if (!profile) {
            profile = new Profile(this);
            profile->setName(names[i]);
            profile->setApp(apps[i]);
            profile->saveProfile();
        }

            profiles.append(profile);
    }

    profileInstance = profiles[0];
    currentProfile = profiles[0];
}

void MainWindow::switchCurrentProfile(const QString& appName) {
    qDebug() << "Current app:" << appName;
    for (Profile* profile : profiles) {
        if (profile->getApp() == appName) {
            currentProfile = profile;
            qDebug() << "Current profile set to:" << currentProfile->getName();
            return;
        }
    }
}
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

static void volumeUp()
{
#ifdef _WIN32
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
    MainWindow::macVolume = (MainWindow::macVolume >= 100) ? MainWindow::macVolume : MainWindow::macVolume + 6;
    setSystemVolume(MainWindow::macVolume);
#endif
}

static void volumeDown()
{
#ifdef _WIN32
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
    MainWindow::macVolume = (MainWindow::macVolume <= 0) ? MainWindow::macVolume : MainWindow::macVolume - 6;
    setSystemVolume(MainWindow::macVolume);
#endif
}

static void mute()
{
#ifdef _WIN32
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
    toggleMuteSystem();
#endif
}


// Scroll functions

static void scrollUp()
{
#ifdef _WIN32
    // qDebug() << "scrollUp called on Windows";
    // QScrollArea* scrollArea = ui->scrollArea;
    // QScrollBar* vScrollBar = scrollArea->verticalScrollBar();
    // if (vScrollBar) {
    //     vScrollBar->setValue(vScrollBar->value() - 50);
    // }
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

static void scrollDown()
{
#ifdef _WIN32
    // qDebug() << "scrollDown called on Windows";
    // QScrollArea* scrollArea = ui->scrollArea;
    // QScrollBar* vScrollBar = scrollArea->verticalScrollBar();
    // if (vScrollBar) {
    //     vScrollBar->setValue(vScrollBar->value() + 50);
    // }
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



// std::string pathNotion = "C:\\Users\\aarav\\AppData\\Local\\Programs\\Notion\\Notion.exe";
// std::wstring wpathNotion(pathNotion.begin(), pathNotion.end());


void MainWindow::onDataReceived(int number)
{
    // qDebug() << "onDataReceived: " << number;
    // if (number > 10 && number < 20)
    // {
    //     profile = number - 10;
    //     qDebug() << "Profile switched to " << profile;
    // }

    //Volume Control
    if (number > 70 && number < 80)
    {
        if (number == 72)
            volumeUp();
        else if (number == 71)
            volumeDown();
        else if (number == 73)
            mute();
        return;
    }
    //if (number > 0 && number < 10) executeHotkey(number);
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

