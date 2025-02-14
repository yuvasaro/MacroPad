#include "profile.h"
#include <iostream>
#include <fstream>

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

Profile::Profile(const string& userName): name(userName) {}

void Profile::setMacro(int keyNum, const string& type, const string& content) {
    macros[keyNum] = Macro(type, content);
    cout << "Macro set for keyNum=" << keyNum << ", content=" << macros[keyNum].getContent() << endl;
}

void Profile::deleteMacro(int keyNum) {
    macros.erase(keyNum);
}

void Profile::runMacro(int keyNum) {
    cout << "In Profile::runMacro type='" << macros[keyNum].getType() << "', content='" << macros[keyNum].getContent() << "'" << endl;
    auto it = macros.find(keyNum);
    if (it != macros.end()) {
        macros[keyNum].runCallback();
    } else {
        cout << "Macro not found!\n";
    }
}

// save profile to file
void Profile::saveProfile(const string& filePath) {
    ofstream outFile(filePath);
    if (outFile.is_open()) {
        outFile << "Name: " << name << "\n";
        for (auto& macro : macros) {
            outFile << macro.first << ":\n";
            outFile << "type: " << macro.second.getType() << "\n";
            outFile << "content: " << macro.second.getContent() << "\n";
        }

        outFile.close();
    } else {
        cerr << "Unable to open file for writing.\n";
    }
}

// Load profile from file
Profile Profile::loadProfile(const string& filePath) {
    ifstream inFile(filePath);

    if(inFile.is_open()) {
        string line;
        string profileName;
        string macroType;
        string macroContent;
        int keyNum = -1;

        getline(inFile, line);
        if (line.rfind("Name: ", 0) == 0) {
            profileName = line.substr(6);
        } else {
            cerr << "Missing profile name!\n";
            return Profile("");
        }

        Profile userProfile(profileName);

        while (getline(inFile, line)) {
            if (isdigit(line[0])) {
                keyNum = stoi(line);
            } else if (line.rfind("type: ") == 0) {
                macroType = line.substr(6);
            } else if (line.rfind("content: ") == 0) {
                macroContent = line.substr(9);
                if(keyNum != -1) {
                    userProfile.setMacro(keyNum, macroType, macroContent);
                    keyNum = -1;
                }
            }
        }
        inFile.close();
        return userProfile;

    } else {
        cerr << "Unable to open file for reading.\n";
        return Profile("");
    }
}
