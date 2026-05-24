import QtQuick
import QtQuick.Controls

Dialog {
    id: keyConfigDialog

    SystemPalette { id: sysPal; colorGroup: SystemPalette.Active }
    readonly property color labelColor: sysPal.windowText
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
    property bool isRecordingKeystroke: false

    function stopActiveRecording() {
        if (isRecordingKeystroke) {
            mainWindow.stopRecording();
            isRecordingKeystroke = false;
        }
    }

    function selectedComboKeystroke() {
        var keys = [];
        if (modifier1.currentText !== "None") keys.push(modifier1.currentText);
        if (modifier2.currentText !== "None") keys.push(modifier2.currentText);
        if (keySelection.currentText !== "None") keys.push(keySelection.currentText);
        return keys.join("+");
    }

    function updateKeystrokeFromCombos() {
        if (isRecordingKeystroke) return;

        var selectedKeystroke = selectedComboKeystroke();
        keystrokeInput.text = selectedKeystroke !== "" ? selectedKeystroke : "Select Keystroke Combination";
    }

    onRejected: stopActiveRecording()
    onClosed: stopActiveRecording()

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
            color: keyConfigDialog.labelColor
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
            color: keyConfigDialog.labelColor
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Text {
            id: keystrokeInput
            text: "Select Keystroke Combination"
            color: keyConfigDialog.labelColor
        }

        Button {
            text: keyConfigDialog.isRecordingKeystroke ? "Stop Recording" : "Record Keystroke"
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                if (keyConfigDialog.isRecordingKeystroke) {
                    var recordedKeystroke = mainWindow.stopRecording();
                    keyConfigDialog.isRecordingKeystroke = false;
                    if (recordedKeystroke !== "") {
                        keystrokeInput.text = recordedKeystroke;
                    }
                } else if (mainWindow.startRecording()) {
                    keyConfigDialog.isRecordingKeystroke = true;
                    keystrokeInput.text = "Recording...";
                }
            }
        }

        Row {
            spacing: 5
            anchors.horizontalCenter: parent.horizontalCenter

            ComboBox {
                id: modifier1
                model: ["None", "Ctrl", "Alt", "Shift", "Win", "Tab", "Cmd", "Fn", "Option", "Caps Lock", "Del", "Enter", "Backspace", "Esc", "Delete", "Return"]
                currentIndex: 0
                onActivated: keyConfigDialog.updateKeystrokeFromCombos()
            }

            ComboBox {
                id: modifier2
                model: ["None", "Ctrl", "Alt", "Shift", "Win", "Tab", "Cmd", "Fn", "Option", "Caps Lock", "Del", "Enter", "Backspace", "Esc", "Delete", "Return"]
                currentIndex: 0
                onActivated: keyConfigDialog.updateKeystrokeFromCombos()
            }

            ComboBox {
                id: keySelection
                model: ["None", "Space", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12"]
                currentIndex: 0
                onActivated: keyConfigDialog.updateKeystrokeFromCombos()
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
                onClicked: {
                    var imagePath = mainWindow.browseImageFile();
                    if (imagePath !== "") {
                        keyConfigDialog.keyImage = "file://" + imagePath;
                        keyImagePreviewKeystroke.source = keyConfigDialog.keyImage;
                        keyImagePreviewExecutable.source = keyConfigDialog.keyImage;
                    }
                }
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
                    keyConfigDialog.stopActiveRecording();
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
                    if (keyConfigDialog.isRecordingKeystroke) {
                        var recordedKeystroke = mainWindow.stopRecording();
                        keyConfigDialog.isRecordingKeystroke = false;
                        if (recordedKeystroke !== "") {
                            keystrokeInput.text = recordedKeystroke;
                        }
                    }

                    var keystrokeValue = keystrokeInput.text;
                    if (keystrokeValue === "Select Keystroke Combination" || keystrokeValue === "Recording...") {
                        keystrokeValue = selectedComboKeystroke();
                    }

                    var customImagePath = "";
                    if (customImageCheckKeystroke.checked && keyImagePreviewKeystroke.source !== "") {
                        customImagePath = keyImagePreviewKeystroke.source.toString();
                    }

                    console.log("Saving key", keyConfigDialog.keyIndex, "Keystroke:", keystrokeValue, "Image:", customImagePath);

                    if (keystrokeValue !== "") {
                        keyConfigDialog.keystroke = keystrokeValue;
                        keyConfigDialog.executable = "";
                        profileManager.setKeyConfig(keyConfigDialog.keyIndex, "keystroke", keystrokeValue, customImagePath);
                        mainWindow.callHotkeyHandler(hotkeyHandler.profileManager, keyConfigDialog.keyIndex, "keystroke", keystrokeValue);
                    }

                    keyConfigDialog.accept();
                }
            }

            Button {
                text: "Cancel"
                onClicked: {
                    keyConfigDialog.stopActiveRecording();
                    keyConfigDialog.reject();
                }
            }

            Button {
                text: "Clear"
                onClicked: {
                    keyConfigDialog.stopActiveRecording();
                    profileManager.clearKeyConfig(keyConfigDialog.keyIndex);
                    keyConfigDialog.keystroke = "";
                    keyConfigDialog.executable = "";
                    keyConfigDialog.keyImage = "";
                    keyConfigDialog.accept();
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
            color: keyConfigDialog.labelColor
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Button {
            text: "Browse for Executable"
            width: 250
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                var selectedPath = mainWindow.browseExecutableFile();
                if (selectedPath !== "") {
                    console.log("Selected executable:", selectedPath)
                    keyConfigDialog.executable = selectedPath;
                    executablePath.text = keyConfigDialog.executable;

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
                onClicked: {
                    var imagePath = mainWindow.browseImageFile();
                    if (imagePath !== "") {
                        keyConfigDialog.keyImage = "file://" + imagePath;
                        keyImagePreviewKeystroke.source = keyConfigDialog.keyImage;
                        keyImagePreviewExecutable.source = keyConfigDialog.keyImage;
                    }
                }
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
                        keyConfigDialog.keystroke = "";
                        keyConfigDialog.executable = executableValue;
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

            Button {
                text: "Clear"
                onClicked: {
                    profileManager.clearKeyConfig(keyConfigDialog.keyIndex);
                    keyConfigDialog.keystroke = "";
                    keyConfigDialog.executable = "";
                    keyConfigDialog.keyImage = "";
                    keyConfigDialog.accept();
                }
            }
        }
    }

}
