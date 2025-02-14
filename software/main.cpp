#include "mainwindow.h"
#include <QApplication>
#include "profile.h"
#include <iostream>



using namespace std;



int main(int argc, char *argv[]) {
    // these work, so the issue is not with programCallback
    Macro testMacro("program", "echo 'This testMacro programCallback works'");
    testMacro.programCallback();

    Macro testMacro2("program", "open -a 'Google Chrome'");
    testMacro2.programCallback();
    // ---------

    //this also works, so the profile is able to be saved to a txt file
    Profile testProfile("testProfile");
    testProfile.setMacro(1, "program", "echo 'Hello World'");
    testProfile.saveProfile("/Users/minjooo/Documents/profile.txt");
    // --------


    // loading a profile object from a txt file also works because when I save it again, the macros are accurate
    Profile loadedProfile = Profile::loadProfile("/Users/minjooo/Documents/profile.txt");
    loadedProfile.saveProfile("/Users/minjooo/Documents/new_profile.txt");

    // however, when I try to runMacro from a loaded profile, type and content is null :(
    loadedProfile.runMacro(1);

    /*QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec(); */
}
