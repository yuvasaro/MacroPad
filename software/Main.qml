import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

Rectangle {
    id: root
    width: 640
    height: 480
    color: "black"

    ProfileManager {
        id: profileManager
    }


    Grid {
        id: keyGrid
        columns: 3
        spacing: 10
        anchors.centerIn: parent

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
                            var profile = profileManager.profiles[profileSelector.currentIndex];

                            var existingKey = profile.keys[index] || { keystroke: "", executable: "" };

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
            model: profileManager.profileNames
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
                profileManager.loadProfile(currentIndex)
            }
        }


        FileDialog {
            id: fileDialog
            title: "Select an Executable File"
            fileMode: FileDialog.OpenFile
            nameFilters: ["Executable Files (*.exe *.app *.sh)", "All Files (*)"]

            onAccepted: {
                console.log("Selected executable:", selectedFile)
                profileManager.setApp(selectedFile.toString().replace("file://", ""));

            }
        }

        Button {
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
    }

