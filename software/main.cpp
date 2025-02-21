#include "mainwindow.h"
#include "macro.h"
#include "profile.h"
#include "config.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    // create a test profile with some macros
    Profile testProfile = Profile("testUser");
    testProfile.setMacro(1, "program", "echo 'Hello World'");
    testProfile.setMacro(2, "program", "/bin/ls");
    testProfile.setMacro(3, "program", "open -a 'Google Chrome'");
    testProfile.setMacro(4, "keystroke", "[keystroke]");

    //save the profile into config
    testProfile.saveProfile(); // now there is no argument to be passed, saveProfile uses getConfigDir to find the config directory and saves it there

    // loading the profile from the config directory using just the name of the user
    Profile loadedProfile = Profile::loadProfile("testUser");

    loadedProfile.runMacro(1);
    loadedProfile.runMacro(2);
    loadedProfile.runMacro(3);

    /*QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec(); */
}
