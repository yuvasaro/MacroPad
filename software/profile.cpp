#include "profile.h"
#include <iostream>
#include <fstream>

using namespace std;

Profile::Profile(const string& userName): name(userName) {}

<<<<<<< Updated upstream
void Profile::setMacro(int keyNum, const string& type, const string& content) {
    macros[keyNum] = std::move(make_unique<Macro>(type, content));
=======
Profile::Profile(const QString& userName, QObject* parent) : QObject(parent), name(userName) {}

QString Profile::getName() const {
    return name;
}

void Profile::setName(const QString& newName) {
    if (name != newName) {
        name = newName;
        emit nameChanged();
    }
}

void Profile::setMacro(int keyNum, const QString& type, const QString& content) {
    macros[keyNum] = QSharedPointer<Macro>::create(type, content);
>>>>>>> Stashed changes
}


void Profile::deleteMacro(int keyNum) {
    macros.remove(keyNum);
}

<<<<<<< Updated upstream
void Profile::runMacro(int keyNum) {
    cout << "In Profile::runMacro type='" << macros[keyNum]->getType() << "', content='" << macros[keyNum]->getContent() << "'" << endl;
    if (macros.find(keyNum) != macros.end()) {
        macros[keyNum]->callback();
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
            outFile << "type: " << macro.second->getType() << "\n";
            outFile << "content: " << macro.second->getContent() << "\n";
=======
QSharedPointer<Macro> Profile::getMacro(int keyNum) {
    return macros.value(keyNum, QSharedPointer<Macro>());
}

void Profile::saveProfile() {
    QString configDir = Config::getConfigDir();
    QString filePath = configDir + "/" + name + ".txt";

    QFile outFile(filePath);
    if (outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&outFile);
        out << "Name: " << name << "\n";

        for (auto it = macros.begin(); it != macros.end(); ++it) {
            out << it.key() << ":\n";
            out << "type: " << it.value()->getType() << "\n";
            out << "content: " << it.value()->getContent() << "\n";
>>>>>>> Stashed changes
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

