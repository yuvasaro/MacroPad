    #include "profile.h"
    #include <iostream>
    #include <fstream>
    #include <memory>
    #include "config.h"
    #include <QString>
    #include <QFile>
    #include <QTextStream>
    #include <QDir>


    Profile::Profile(QObject* parent) : QObject(parent) {}

    Profile::Profile(const QString& profileName, const QString& appName, QObject* parent) : QObject(parent), app(appName), name(profileName) {}

    QString Profile::getName() const {
        return name;
    }

    QString Profile::getApp() const {
        return app;
    }

    void Profile::setName(const QString& newName) {
        if (name != newName) {
            name = newName;
            emit nameChanged();
        }
    }

    void Profile::setApp(const QString& newApp) {
        if (app != newApp) {
            app = newApp;
            emit appChanged();
        }
    }

    void Profile::setMacro(int keyNum, const QString& type, const QString& content) {
        if (!macros.contains(keyNum)) {
            macros[keyNum] = QSharedPointer<Macro>::create();
        }

        macros[keyNum]->setType(type);
        macros[keyNum]->setContent(content);

    }

    void Profile::deleteMacro(int keyNum) {
        macros.remove(keyNum);
    }

    QSharedPointer<Macro> Profile::getMacro(int keyNum) {
        return macros.value(keyNum, QSharedPointer<Macro>());
    }

    void Profile::saveProfile() {
        qDebug() << "Saving profile. Current macros:";
        this->printMacros();
        QString configDir = Config::getConfigDir();
        QString filePath = configDir + "/" + name + ".txt";

        QFile outFile(filePath);
        if (outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&outFile);
            out << "Name: " << name << "\n";
            out << "App: " << app << "\n";

            for (auto it = macros.begin(); it != macros.end(); ++it) {
                out << it.key() << ":\n";
                out << "type: " << it.value()->getType() << "\n";
                out << "content: " << it.value()->getContent() << "\n";
            }

            outFile.close();
        } else {
            qCritical() << "Unable to open file for writing:" << filePath;
        }
    }

    // reads the .txt file in config with the matching name and creates its profile and macro objects
    Profile* Profile::loadProfile(const QString& nameLookUp) {

        qDebug() << "Loading profile: " << nameLookUp;
        QString filePath = Config::getConfigDir() + "/" + nameLookUp + ".txt";
        QFile inFile(filePath);

        if (inFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&inFile);
            QString line;
            QString profileName;
            QString profileApp;
            QString macroType;
            QString macroContent;
            int keyNum = -1;
            int macroCount = 0;

            line = in.readLine();
            if (line.startsWith("Name: ")) {
                profileName = line.mid(6);
            } else {
                qWarning() << "Missing profile name!";
                return new Profile("", "", nullptr);
            }

            line = in.readLine();
            if (line.startsWith("App: ")) {
                profileApp = line.mid(5);
            } else {
                qWarning() << "Missing profile name!";
                return new Profile("", "", nullptr);            ;
            }

            Profile* userProfile = new Profile(profileName, profileApp);

            if (line.startsWith("Application: ")){
                profileApp = line.mid(13);
            }

            while (!in.atEnd()) {
                line = in.readLine().trimmed();
                qDebug() << "  >> line:" << line;

                // 1) Is it an index line?  It must:
                //    • end with “:”
                //    • the part before “:” must be a valid integer string (e.g. “-2”, “0”, “9”)
                //    • and that integer must lie between -2 and 9
                if (line.endsWith(":")) {
                    QString numStr = line.left(line.length() - 1);
                    bool ok = false;
                    int n = numStr.toInt(&ok);

                    // ensure toInt succeeded AND numStr has no extra chars (no “content”!)
                    if (ok && numStr == QString::number(n) && n >= -2 && n <= 9) {
                        keyNum = n;
                        qDebug() << "    Detected key index =" << keyNum;
                    } else {
                        qWarning() << "    [Ignored non-numeric or out-of-range index]" << numStr;
                        keyNum = -1;
                    }
                    continue;
                }

                // 2) type: …
                if (line.startsWith("type: ")) {
                    macroType = line.mid(6).trimmed();
                    qDebug() << "    Detected type =" << macroType;
                    continue;
                }

                // 3) content: …
                if (line.startsWith("content: ")) {
                    macroContent = line.mid(9).trimmed();
                    qDebug() << "    Detected content =" << macroContent;

                    if (keyNum != -1 && !macroType.isEmpty() && !macroContent.isEmpty()) {
                        userProfile->setMacro(keyNum, macroType, macroContent);
                        qDebug() << "      [Loaded Macro]"
                                 << "key =" << keyNum
                                 << ", type =" << macroType
                                 << ", content =" << macroContent;
                        ++macroCount;
                    } else {
                        qWarning() << "      [Skipped malformed macro] key:" << keyNum
                                   << " type:" << macroType
                                   << " content:" << macroContent;
                    }

                    // reset for the next block
                    keyNum = -1;
                    macroType.clear();
                    macroContent.clear();
                    continue;
                }
            }

            inFile.close();
            return userProfile;

        } else {
            qWarning() << "Unable to open file for reading:" << filePath;
            return nullptr;
        }

    }

    void Profile::printMacros() {
        for (auto it = macros.constBegin(); it != macros.constEnd(); ++it) {
            qDebug() << "Key:" << it.key() << "Macro:" << it.value()->toString();
        }
    }
