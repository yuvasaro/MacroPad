import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

Rectangle {
    id: root
    width: 940
    height: 850
    color: "black"

    Component.onCompleted: {
        var num = hotkeyHandler.profiles.length;
        console.log("Number of profiles from mainwindow:", num);
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

                Text {
                    text: "Key " + (index + 1)
                    anchors.centerIn: parent
                    color: "black"
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
                onCurrentIndexChanged: {
                    hotkeyHandler.profileManager= hotkeyHandler.profiles[currentIndex];
                }
                console.log("Selected profile:", hotkeyHandler.profileManager.name);

            }
        }

        FileDialog {
            id: fileDialog
            title: "Select an Executable File"
            fileMode: FileDialog.OpenFile
            nameFilters: ["Executable Files (*.exe *.app *.sh)", "All Files (*)"]

            onAccepted: {
                console.log("Selected executable:", selectedFile)
                let fullPath = selectedFile.toString().replace("file://", "");
                let appName = fullPath.split("/").pop().replace(".exe", "").replace(".app", "").replace(".sh", "");
                profileManager.setApp(appName);
                exetext.text = appName;
                // profileInstance.saveProfile();
            }
        }

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

