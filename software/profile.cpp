#include "profile.h"
#include <iostream>
#include <fstream>
#include <memory>
#include "config.h"

using namespace std;

Profile::Profile(const string& userName): name(userName) {}

void Profile::setMacro(int keyNum, const string& type, const string& content) {
    macros[keyNum] = std::move(std::make_unique<Macro>(type, content));
}

void Profile::deleteMacro(int keyNum) {
    macros.erase(keyNum);
}

void Profile::runMacro(int keyNum) {
    if (macros.find(keyNum) != macros.end()) {
        macros[keyNum]->callback();
    } else {
        cout << "Macro not found!\n";
    }
}

// save profile to file
void Profile::saveProfile() {
    filesystem::path configDir = Config::getConfigDir();
    filesystem::path filePath = configDir / (name + ".txt");

    ofstream outFile(filePath);
    if (outFile.is_open()) {
        outFile << "Name: " << name << "\n";
        for (auto& macro : macros) {
            outFile << macro.first << ":\n";
            outFile << "type: " << macro.second->getType() << "\n";
            outFile << "content: " << macro.second->getContent() << "\n";
        }

        outFile.close();
    } else {
        cerr << "Unable to open file for writing.\n";
    }
}

// Load profile from file
Profile Profile::loadProfile(const string& nameLookUp) {
    filesystem::path filePath = Config::getConfigDir() / (nameLookUp + ".txt");
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

