import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import IconExtractor 1.0

Dialog {
    id: keyConfigDialog
    width: 450
    height: 350
    modal: true
    title: "Configure Key " + keyIndex

    property int keyIndex: 0
    property string keystroke: ""
    property string executable: ""
    property string keyImage: ""
    property bool useCustomImage: false
    signal accepted()
    property string customImage: ""
    property string extractedIconPath: ""
    property string configMode: "select"
    property string recordedCombo: ""
    property string nameLabel: ""

    Connections {
        target: hotkeyHandler
        function onRecordedKeyChanged(combo) {
            keyConfigDialog.recordedCombo = combo;
        }
    }

    IconExtractor {
        id: iconExtractor
    }

    Component.onCompleted: {
        console.log("Initializing KeyConfig Dialog for Key:", keyIndex);
        if (keystroke !== "" && keystroke !== undefined) {
            configMode = "keystroke";
            recordedCombo = keystroke;
            var macroData = hotkeyHandler.profileManager.getMacroData(keyIndex);
            if (macroData.label && macroData.label !== "") {
                nameField.text = macroData.label;
            }
        } else if (executable !== "" && executable !== undefined) {
            configMode = "executable";
        } else {
            configMode = "select";
        }
    }

    Column {
        anchors.centerIn: parent
        spacing: 20
        visible: configMode === "select"

        Text {
            text: "What would you like to configure?"
            font.pixelSize: 16
            font.bold: true
            color: "grey"
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Button {
            text: "Keystroke Combination"
            width: 250
            height: 60
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                configMode = "keystroke";
            }
        }

        Button {
            text: "Executable Path"
            width: 250
            height: 60
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                configMode = "executable";
            }
        }

        Button {
            text: "Cancel"
            width: 250
            height: 40
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: keyConfigDialog.reject();
        }
    }

    Column {
        anchors.centerIn: parent
        spacing: 12
        visible: configMode === "keystroke"

        Text {
            text: "Configure Keystroke Combination"
            font.pixelSize: 14
            font.bold: true
            color: "grey"
            anchors.horizontalCenter: parent.horizontalCenter
        }

        // Name / label field
        TextField {
            id: nameField
            width: 300
            placeholderText: "Name this shortcut (e.g. Save File)"
            anchors.horizontalCenter: parent.horizontalCenter
        }

        // Live combo display
        Rectangle {
            width: 300
            height: 40
            color: "#222"
            radius: 6
            border.color: hotkeyHandler.isRecording ? "#ff4444" : "#555"
            anchors.horizontalCenter: parent.horizontalCenter

            Text {
                id: comboDisplay
                anchors.centerIn: parent
                color: hotkeyHandler.isRecording ? "#ff8888" : "white"
                text: {
                    if (hotkeyHandler.isRecording) return recordedCombo !== "" ? recordedCombo : "Press keys..."
                    return recordedCombo !== "" ? recordedCombo : "No keys recorded"
                }
            }
        }

        // Record / Stop buttons
        Row {
            spacing: 10
            anchors.horizontalCenter: parent.horizontalCenter

            Button {
                text: "Record"
                enabled: !hotkeyHandler.isRecording
                onClicked: {
                    recordedCombo = "";
                    hotkeyHandler.startRecording();
                }
            }

            Button {
                text: "Stop"
                enabled: hotkeyHandler.isRecording
                onClicked: {
                    recordedCombo = hotkeyHandler.stopRecording();
                }
            }
        }

        Row {
            spacing: 10
            anchors.horizontalCenter: parent.horizontalCenter

            CheckBox {
                id: customImageCheckKeystroke
                text: "Use custom image"
                checked: keyConfigDialog.useCustomImage
                onCheckedChanged: keyConfigDialog.useCustomImage = checked
            }

            Button {
                text: "Browse Image"
                enabled: customImageCheckKeystroke.checked
                onClicked: imageDialog.open()
            }
        }

        Image {
            id: keyImagePreviewKeystroke
            width: 50
            height: 50
            source: keyConfigDialog.keyImage
            visible: source !== ""
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Row {
            spacing: 10
            anchors.horizontalCenter: parent.horizontalCenter

            Button {
                text: "Back"
                onClicked: {
                    if (hotkeyHandler.isRecording) hotkeyHandler.stopRecording();
                    recordedCombo = "";
                    configMode = "select";
                }
            }

            Button {
                text: "Save"
                enabled: recordedCombo !== ""
                onClicked: {
                    if (hotkeyHandler.isRecording) {
                        recordedCombo = hotkeyHandler.stopRecording();
                    }
                    var customImagePath = "";
                    if (customImageCheckKeystroke.checked && keyImagePreviewKeystroke.source !== "") {
                        customImagePath = keyImagePreviewKeystroke.source.toString();
                    }
                    if (recordedCombo !== "") {
                        profileManager.setKeyConfig(keyConfigDialog.keyIndex, "keystroke", recordedCombo, customImagePath, nameField.text);
                        mainWindow.callHotkeyHandler(hotkeyHandler.profileManager, keyConfigDialog.keyIndex, "keystroke", recordedCombo);
                    }
                    keyConfigDialog.accept();
                }
            }

            Button {
                text: "Cancel"
                onClicked: {
                    if (hotkeyHandler.isRecording) hotkeyHandler.stopRecording();
                    keyConfigDialog.reject();
                }
            }
        }
    }

    Column {
            anchors.centerIn: parent
            spacing: 10
            visible: configMode === "executable"

            Text {
                text: "Configure Executable"
                font.pixelSize: 14
                font.bold: true
                color: "grey"
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Button {
                text: "Browse for Executable"
                width: 250
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: fileDialog.open()
            }

            TextField {
                id: executablePath
                width: 300
                placeholderText: "Selected Executable Path"
                text: keyConfigDialog.executable
                readOnly: true
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Row {
                spacing: 10
                visible: executablePath.text !== ""
                anchors.horizontalCenter: parent.horizontalCenter

                CheckBox {
                    id: customImageCheckExecutable
                    text: "Use custom image"
                    checked: keyConfigDialog.useCustomImage
                    onCheckedChanged: keyConfigDialog.useCustomImage = checked
                }

                Button {
                    text: "Browse Image"
                    enabled: customImageCheckExecutable.checked
                    onClicked: imageDialog.open()
                }
            }

            Image {
                id: keyImagePreviewExecutable
                width: 50
                height: 50
                source: keyConfigDialog.keyImage
                visible: source !== ""
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Row {
                spacing: 10
                anchors.horizontalCenter: parent.horizontalCenter

                Button {
                    text: "Back"
                    onClicked: {
                        configMode = "select";
                        executablePath.text = "";
                    }
                }

                Button {
                    text: "Save"
                    onClicked: {
                        var executableValue = executablePath.text;

                        // Use custom image if checked, otherwise use extracted icon
                        var imageValue = customImageCheckExecutable.checked ? keyImagePreviewExecutable.source.toString() : extractedIconPath;

                        console.log("Saving key", keyConfigDialog.keyIndex, "Executable:", executableValue, "Image:", imageValue);

                        if (executableValue !== "") {
                            // Pass the icon path (either extracted or custom)
                            profileManager.setKeyConfig(keyConfigDialog.keyIndex, "executable", executableValue, imageValue);
                            mainWindow.callHotkeyHandler(hotkeyHandler.profileManager, keyConfigDialog.keyIndex, "executable", executableValue);
                        }

                        keyConfigDialog.accept();
                    }
                }

                Button {
                    text: "Cancel"
                    onClicked: keyConfigDialog.reject();
                }
            }
        }

    FileDialog {
        id: imageDialog
        title: "Select Key Image"
        fileMode: FileDialog.OpenFile
        nameFilters: ["Image Files (*.png *.jpg *.jpeg)", "All Files (*)"]

        onAccepted: {
            keyConfigDialog.keyImage = selectedFile.toString();
            keyImagePreviewKeystroke.source = keyConfigDialog.keyImage;
            keyImagePreviewExecutable.source = keyConfigDialog.keyImage;
        }
    }

    FileDialog {
        id: fileDialog
        title: "Select an Executable File"
        fileMode: FileDialog.OpenFile
        nameFilters: ["Executable Files (*.exe *.app *.sh)", "All Files (*)"]

        onAccepted: {
                    console.log("Selected executable:", selectedFile)
                    keyConfigDialog.executable = selectedFile.toString().replace("file://", "");
                    executablePath.text = keyConfigDialog.executable;

                    var iconPath = iconExtractor.extractIconForApp(keyConfigDialog.executable);
                    if (iconPath !== "") {
                        extractedIconPath = "file:///" + iconPath;
                        console.log("Extracted icon path:", extractedIconPath);
                    }


                }
        }
}
