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

    Component.onCompleted: {
        console.log("Initializing KeyConfig Dialog for Key:", keyIndex);
        keystrokeInput.text = keystroke;
        executablePath.text = executable;
    }

    Column {
        anchors.centerIn: parent
        spacing: 10

        Text {
            id: keystrokeInput
            text: "Select Keystroke Combination"
            color: "grey"
        }

        Row {
            spacing: 5

            ComboBox {
                id: modifier1
                model: ["None", "Ctrl", "Alt", "Shift", "Win", "Tab", "Cmd", "Fn", "Option", "Caps Lock", "Del", "Enter", "Backspace", "Esc", "Delete", "Return"]
                currentIndex: 0
                enabled: executablePath.text === ""

                onActivated: {
                    if (currentText !== "None") {
                        executablePath.text = "";
                    }
                }
            }

            ComboBox {
                id: modifier2
                model: ["None", "Ctrl", "Alt", "Shift", "Win", "Tab", "Cmd", "Fn", "Option", "Caps Lock", "Del", "Enter", "Backspace", "Esc", "Delete", "Return"]
                currentIndex: 0
                enabled: executablePath.text === ""

                onActivated: {
                    if (currentText !== "None") {
                        executablePath.text = "";
                    }
                }
            }

            ComboBox {
                id: keySelection
                model: ["None", "Space", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12"]
                currentIndex: 0
                enabled: executablePath.text === ""

                onActivated: {
                    if (currentText !== "None") {
                        executablePath.text = "";
                    }
                }
            }

        }

        Button {
            text: "Browse for Executable"
            enabled: modifier1.currentText === "None" && modifier2.currentText === "None" && keySelection.currentText === "None"  // Disable if keystroke is selected
            onClicked: fileDialog.open()
        }

        TextField {
            id: executablePath
            width: 300
            placeholderText: "Selected Executable Path"
            text: keyConfigDialog.executable
            readOnly: true
        }

        Row {
            spacing: 10
            visible: executablePath.text !== "" || keystrokeInput.text !== ""

            CheckBox {
                id: customImageCheck
                text: "Use custom image"
                checked: keyConfigDialog.useCustomImage
                onCheckedChanged: keyConfigDialog.useCustomImage = checked
            }

            Button {
                text: "Browse Image"
                enabled: customImageCheck.checked
                onClicked: imageDialog.open()
            }
        }

        Image {
            id: keyImagePreview
            width: 50
            height: 50
            source: keyConfigDialog.keyImage
            visible: source !== ""
        }

        FileDialog {
            id: imageDialog
            title: "Select Key Image"
            fileMode: FileDialog.OpenFile
            nameFilters: ["Image Files (*.png *.jpg *.jpeg)", "All Files (*)"]

            onAccepted: {
                keyConfigDialog.keyImage = selectedFile.toString()
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
                var executableValue = executablePath.text;
                var imageValue = customImageCheck.checked ? keyImagePreview.source : "";
                var customImagePath = "";
                        if (customImageCheck.checked && keyImagePreview.source !== "") {
                            customImagePath = keyImagePreview.source.toString();
                        }

                console.log("Saving key", keyConfigDialog.keyIndex, "Keystroke:", keystrokeValue, "Executable:", executableValue, "Image:", imageValue);

                if (keystrokeValue !== "") {
                    profileManager.setKeyConfig(keyConfigDialog.keyIndex, "keystroke", keystrokeValue, imageValue);
                    mainWindow.callHotkeyHandler(hotkeyHandler.profileManager, keyConfigDialog.keyIndex, "keystroke", keystrokeValue);
                }

                if (executableValue !== "") {
                    profileManager.setKeyConfig(keyConfigDialog.keyIndex, "executable", executableValue, customImagePath);
                    mainWindow.callHotkeyHandler(hotkeyHandler.profileManager, keyConfigDialog.keyIndex, "executable", executableValue);
                }

                if (imageValue !== "") {
                        profileManager.setKeyConfig(keyConfigDialog.keyIndex, "image", imageValue);
                    }
                keyConfigDialog.accept();
            }
        }


        Button {
            text: "Cancel"
            onClicked: keyConfigDialog.reject();
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

            modifier1.currentIndex = 0;
            modifier2.currentIndex = 0;
            keySelection.currentIndex = 0;
        }
    }
}

