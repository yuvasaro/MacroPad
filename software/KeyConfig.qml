import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

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

    Component.onCompleted: {
        console.log("Initializing KeyConfig Dialog for Key:", keyIndex);
        if (keystroke !== "" && keystroke !== undefined) {
            configMode = "keystroke";
            keystrokeInput.text = keystroke;
        } else if (executable !== "" && executable !== undefined) {
            configMode = "executable";
            executablePath.text = executable;
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
        spacing: 10
        visible: configMode === "keystroke"

        Text {
            text: "Configure Keystroke Combination"
            font.pixelSize: 14
            font.bold: true
            color: "grey"
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Text {
            id: keystrokeInput
            text: "Select Keystroke Combination"
            color: "grey"
        }

        Row {
            spacing: 5
            anchors.horizontalCenter: parent.horizontalCenter

            ComboBox {
                id: modifier1
                model: ["None", "Ctrl", "Alt", "Shift", "Win", "Tab", "Cmd", "Fn", "Option", "Caps Lock", "Del", "Enter", "Backspace", "Esc", "Delete", "Return"]
                currentIndex: 0
            }

            ComboBox {
                id: modifier2
                model: ["None", "Ctrl", "Alt", "Shift", "Win", "Tab", "Cmd", "Fn", "Option", "Caps Lock", "Del", "Enter", "Backspace", "Esc", "Delete", "Return"]
                currentIndex: 0
            }

            ComboBox {
                id: keySelection
                model: ["None", "Space", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12"]
                currentIndex: 0
            }
        }

        Row {
            spacing: 10
            visible: modifier1.currentText !== "None" || modifier2.currentText !== "None" || keySelection.currentText !== "None"
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
                    configMode = "select";
                    // Reset selections
                    modifier1.currentIndex = 0;
                    modifier2.currentIndex = 0;
                    keySelection.currentIndex = 0;
                }
            }

            Button {
                text: "Save"
                onClicked: {
                    var keys = [];
                    if (modifier1.currentText !== "None") keys.push(modifier1.currentText);
                    if (modifier2.currentText !== "None") keys.push(modifier2.currentText);
                    if (keySelection.currentText !== "None") keys.push(keySelection.currentText);

                    var keystrokeValue = keys.join("+");
                    var customImagePath = "";
                    if (customImageCheckKeystroke.checked && keyImagePreviewKeystroke.source !== "") {
                        customImagePath = keyImagePreviewKeystroke.source.toString();
                    }

                    console.log("Saving key", keyConfigDialog.keyIndex, "Keystroke:", keystrokeValue, "Image:", customImagePath);

                    if (keystrokeValue !== "") {
                        profileManager.setKeyConfig(keyConfigDialog.keyIndex, "keystroke", keystrokeValue, customImagePath);
                        mainWindow.callHotkeyHandler(hotkeyHandler.profileManager, keyConfigDialog.keyIndex, "keystroke", keystrokeValue);
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

                    // Extract icon for the selected executable
                    var iconPath = iconExtractor.extractIconForApp(keyConfigDialog.executable);
                    if (iconPath !== "") {
                        extractedIconPath = "file:///" + iconPath;
                        console.log("Extracted icon path:", extractedIconPath);
                    }

                    modifier1.currentIndex = 0;
                    modifier2.currentIndex = 0;
                    keySelection.currentIndex = 0;
                }
        }
}
