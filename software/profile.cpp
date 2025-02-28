#include "profile.h"
#include "config.h"
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDir>


Profile::Profile(QObject* parent) : QObject(parent) {}

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
    macros[keyNum] = std::move(std::make_unique<Macro>(type, content));
}

void Profile::deleteMacro(int keyNum) {
    macros.erase(keyNum);
}

std::unique_ptr<Macro>& Profile::getMacro(int keyNum) {
    return macros[keyNum];
}

// save profile to file
void Profile::saveProfile() {
    QString configDir = Config::getConfigDir(); // Assuming this method returns a QString
    QString filePath = configDir + "/" + name + ".txt";

    QFile outFile(filePath);
    if (outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&outFile);
        out << "Name: " << name << "\n";

        for (auto& macro : macros) {
            out << macro.first << ":\n";
            out << "type: " << macro.second->getType() << "\n";
            out << "content: " << macro.second->getContent() << "\n";
        }

        outFile.close();
    } else {
        qCritical() << "Unable to open file for writing:" << filePath;
    }
}

// Load profile from file
Profile* Profile::loadProfile(const QString& nameLookUp) {
    QString filePath = Config::getConfigDir() + "/" + nameLookUp + ".txt";
    QFile inFile(filePath);

    if (inFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&inFile);
        QString line;
        QString profileName;
        QString macroType;
        QString macroContent;
        int keyNum = -1;

        // Read the first line for the profile name
        line = in.readLine();
        if (line.startsWith("Name: ")) {
            profileName = line.mid(6); // Skip "Name: "
        } else {
            qWarning() << "Missing profile name!";
            return new Profile(""); // Return empty profile if name is missing
        }

        Profile* userProfile = new Profile(profileName);

        // Read the rest of the file
        while (!in.atEnd()) {
            line = in.readLine();

            // Check if line starts with a number (keyNum)
            if (line[0].isDigit()) {
                keyNum = line.toInt();
            }
            // Check for "type: "
            else if (line.startsWith("type: ")) {
                macroType = line.mid(6); // Skip "type: "
            }
            // Check for "content: "
            else if (line.startsWith("content: ")) {
                macroContent = line.mid(9); // Skip "content: "
                if (keyNum != -1) {
                    userProfile->setMacro(keyNum, macroType, macroContent);
                    keyNum = -1; // Reset keyNum after setting macro
                }
            }
        }

        inFile.close();
        return userProfile;

    } else {
        qWarning() << "Unable to open file for reading:" << filePath;
        return new Profile(""); // Return empty profile if file cannot be opened
    }
}

