#include "mainwindow.h"
#include <QApplication>
#include "profile.h"
#include <iostream>



using namespace std;



int main(int argc, char *argv[]) {
    // these work, so the issue is not with programCallback
    Macro testMacro("program", "echo 'This testMacro programCallback works'");
    testMacro.callback();

    Macro testMacro2("program", "open -a 'Google Chrome'");
    testMacro2.callback();
    // ---------

    //this also works, so the profile is able to be saved to a txt file
    Profile testProfile("testerProfile");
    testProfile.setMacro(1, "program", "echo 'Hello World'");
    testProfile.setMacro(2, "program", "/bin/ls");
    testProfile.setMacro(3, "keystroke", "[keystroke]");
    testProfile.saveProfile("/Users/minjooo/Documents/profile.txt");

    //runMacro from a profile doesn't work!
    testProfile.runMacro(1);
    testProfile.runMacro(2);
    // --------


    // loading a profile object from a txt file also works because when I save it again, the macros are accurate
    Profile loadedProfile = Profile::loadProfile("/Users/minjooo/Documents/profile.txt");
    loadedProfile.saveProfile("/Users/minjooo/Documents/new_profile.txt");

    loadedProfile.runMacro(1);


    /*QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec(); */
}
