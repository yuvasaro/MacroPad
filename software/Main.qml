import QtQuick
import QtQuick.Controls

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

                Item {
                            anchors.fill: parent
                            anchors.margins: 5

                            Image {
                                id: keyImage
                                anchors.centerIn: parent
                                width: parent.width - 10
                                height: parent.height - 10
                                fillMode: Image.PreserveAspectFit
                                source: {
                                    var profile = profileManager.profiles[profileSelector.currentIndex];
                                    if (profile && profile.keys[index] && profile.keys[index].image) {
                                        return profile.keys[index].image;
                                    }
                                    return "";
                                }
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
                                    keyConfigInstance.executable !== "" ? "executable" : "keystroke",
                                    keyConfigInstance.executable !== "" ? keyConfigInstance.executable : keyConfigInstance.keystroke,
                                    keyConfigInstance.customImage
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
    }

