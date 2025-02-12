#include "profile.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
/*
class Profile:
  name: String
  macros: Dictionary of {int, Macro}

  Profile(userName):
    name = userName

  void setMacro(keyNum, type, content):
    macros[keyNum] = new Macro(type, content)

  void deleteMacro(keyNum):
    macros[keyNum] = NULL

  void runMacro(keyNum)
    macros[keynum].callback()

  // Save to and load from files, develop format
  void saveProfile(filePath):
    // TODO: save profile to file

  static Profile loadProfile(filePath):
    // TODO: load profile from file
 */

Profile::Profile(const string& userName) {

}

void Profile::setMacro(int keyNum, string& type, string& content) {
    macros[keyNum] = new Macro(type, content);
}

void Profile::deleteMacro(int keyNum) {
    macros.erase(keyNum);
}

void Profile::runMacro(int keyNum) {
    if (macros.find(keyNum) != macros.end()) {
        macros[keyNum]->runCallback();
    } else {
        std::cout << "Macro not found!\n";
    }
}

// Save to file (not implemented yet)
void Profile::saveProfile(string& filePath) {
    std::ofstream outFile(filePath);
    if (outFile.is_open()) {
        outFile << "Name: " << name << "\n";
        for (auto& macro : macros) {
            outFile << macro.first << ":\n";
            outFile << "  type: " << macro.second->getType() << "\n";
            outFile << "  content: " << macro.second->getContent() << "\n";
        }
        outFile.close();
    } else {
        std::cerr << "Unable to open file for writing.\n";
    }
}

// Load profile from file
Profile Profile::loadProfile(string&) {

}
