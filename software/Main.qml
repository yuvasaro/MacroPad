import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    width: 940
    height: 850
    color: "black"

    Component.onCompleted: {
        var num = mainWindow.profiles.length;
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
                            var profile = mainWindow.profileInstance;
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
            model: mainWindow.profiles
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
                    mainWindow.profileInstance = mainWindow.profiles[currentIndex];
                }
                console.log("Selected profile:", mainWindow.profileInstance.name);

            }
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

