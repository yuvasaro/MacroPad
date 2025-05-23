import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import IconExtractor 1.0

Rectangle {
    id: root
    width: 940
    height: 850
    color: "black"
    property string exeIconPath: ""

    Component.onCompleted: {
        var num = hotkeyHandler.profiles.length;
        console.log("Number of profiles from mainwindow:", num);
    }

    IconExtractor {
        id: iconExtractor
    }

    ProfileManager {
        id: profileManager
    }

    Grid {
        id: keyGrid
        columns: 3
        spacing: 10
        anchors.top: encoderConfigBox.bottom
        anchors.topMargin: 40
        anchors.horizontalCenter: parent.horizontalCenter

        Repeater {
            model: 9
            Rectangle {
                width: 100
                height: 100
                color: "lightgray"
                border.color: "white"
                radius: 10

                Item {
                        anchors.fill: parent
                        anchors.margins: 5

                        Image {
                            id: keyImage
                            anchors.centerIn: parent
                            width: parent.width - 10
                            height: parent.height - 10
                            fillMode: Image.PreserveAspectFit
                            source: hotkeyHandler.profileManager.getMacroImagePath(index + 1)
                            visible: source !== ""
                        }

                        Text {
                            text: "Key " + (index + 1)
                            anchors.centerIn: parent
                            color: "black"
                            visible: !keyImage.visible
                        }
                    }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        var component = Qt.createComponent("KeyConfig.qml");
                        if (component.status === Component.Ready) {
                            var profile = hotkeyHandler.profileManager;
                            var macro = profile.getMacro(index + 1);
                            var existingKey = macro ? { keystroke: macro.type === "keystroke" ? macro.content : "",
                                                        executable: macro.type === "executable" ? macro.content: "" }
                                                    : { keystroke: "", executable: ""};

                            var keyConfigInstance = component.createObject(root, {
                                keyIndex: index + 1,
                                keystroke: existingKey.keystroke,
                                executable: existingKey.executable
                            });

                            keyConfigInstance.accepted.connect(function () {
                                console.log("Saving key", keyConfigInstance.keyIndex,
                                            "Keystroke:", keyConfigInstance.keystroke,
                                            "Executable:", keyConfigInstance.executable);

                                profileManager.setKeyConfig(
                                    keyConfigInstance.keyIndex,
                                    "keystroke",
                                    keyConfigInstance.keystroke
                                );
                                profileManager.setKeyConfig(
                                    keyConfigInstance.keyIndex,
                                    "executable",
                                    keyConfigInstance.executable
                                );
                                profileManager.setKeyConfig(
                                    keyConfigInstance.customImage);

                                keyConfigInstance.destroy();
                            });

                            keyConfigInstance.rejected.connect(function () {
                                keyConfigInstance.destroy();
                            });

                            keyConfigInstance.open();
                        }
                    }

                }
            }
        }
    }

        KeyConfig {
            id: keyConfigDialog
        }

        ComboBox {
            id: profileSelector
            anchors.top: parent.top
            anchors.topMargin: 20
            anchors.horizontalCenter: parent.horizontalCenter
            model: hotkeyHandler.profiles
            textRole: "name"
            background: Rectangle {
                    color: "lightgray"
                    radius: 5
                    border.color: "white"
                }
            contentItem: Text {
                        text: parent.displayText
                        color: "black"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
            onCurrentIndexChanged: {
                    hotkeyHandler.profileManager= hotkeyHandler.profiles[currentIndex];
                    exetext.text = hotkeyHandler.profileManager.getApp();
                    console.log("Selected profile:", hotkeyHandler.profileManager.name);
            }
        }

// ---------- app selection logic --------
        FileDialog {
            id: fileDialog
            title: "Select an Executable File"
            fileMode: FileDialog.OpenFile
            nameFilters: ["Executable Files (*.exe *.app *.sh)", "All Files (*)"]

            onAccepted: {
                console.log("Selected executable:", selectedFile)
                let fullPath = selectedFile.toString().replace("file://", "");

                // extract the app name from the full path so the app tracker can match it to just the name
                let appName = fullPath.split("/").pop().replace(".exe", "").replace(".app", "").replace(".sh", "");

                // makes sure to set the appname of the selected profile and save it
                profileManager.setApp(appName);

                // displays the name of the app for the selected profile in the UI
                exetext.text = hotkeyHandler.profileManager.getApp();

                let iconPath = iconExtractor.extractIconForApp(fullPath);
                exeIconPath = iconPath !== "" ? "file://" + iconPath : "";
            }
        }

        // button to select the app for the profile
        Button {
            id: exebutton
            width: 100
            height: 40
            text: "Select Executable"
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: 10
            anchors.rightMargin: 10
            visible: profileSelector.currentText !== "General"
            onClicked: fileDialog.open()
        }

        // where the name of the selected app for the profile is displayed
        Button {
            id:exetext
            width: 100
            height: 40
            anchors.top: exebutton.bottom
            anchors.topMargin: 10
            anchors.right: exebutton.right
            visible: profileSelector.currentText !== "General"
            ToolTip.visible: hovered
                ToolTip.text: text
                ToolTip.delay: 500
        }

        Image {
            id:exeIcon
            width: 40
            height: 40
            anchors.top: exetext.bottom
            anchors.topMargin: 10
            anchors.horizontalCenter: exetext.horizontalCenter
            fillMode: Image.PreserveAspectFit
            source: exeIconPath
            visible: profileSelector.currentText !== "General"
        }
// -----------------------------------

        GroupBox {
                    id: encoderConfigBox
                    title: "Rotary Encoder Actions"
                    anchors.top: profileSelector.bottom
                    anchors.topMargin: 20
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 400

                    Column {
                        spacing: 10
                        padding: 10

                        Row {
                            spacing: 10
                            Text { text: "Encoder 1:"; width: 100; color: "white" }

                            ComboBox {
                                id: encoder1Combo
                                model: ["None", "Scroll", "Volume", "Chrome Tabs", "Switch Apps", "Brightness", "Zoom"]
                                width: 200
                                onCurrentTextChanged: {
                                    profileManager.setKeyConfig(-1, "encoder1", currentText);
                                }
                            }
                        }

                        Row {
                            spacing: 10
                            Text { text: "Encoder 2:"; width: 100; color: "white" }

                            ComboBox {
                                id: encoder2Combo
                                model: ["None", "Scroll", "Volume", "Chrome Tabs", "Switch Apps", "Brightness", "Zoom"]
                                width: 200
                                onCurrentTextChanged: {
                                    profileManager.setKeyConfig(-2, "encoder2", currentText);
                                }
                            }
                        }
                    }
                }
    }
