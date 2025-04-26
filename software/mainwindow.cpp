// mainwindow.cpp (refactored)
#include "mainwindow.h"
#include "fileio.h"
#include "profile.h"
#include "hotkeyhandler.h"
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



// Profile* MainWindow::profileManager = new Profile(NULL);
QList<Profile*> profiles;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), trayIcon(new QSystemTrayIcon(this)), trayMenu(new QMenu(this)) {

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

