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
    property int refreshCounter: 0

    Component.onCompleted: {
        if (!hotkeyHandler || !hotkeyHandler.profiles) return;
            var num = hotkeyHandler.profiles.length;
            console.log("Number of profiles from mainwindow:", num);
            // Force initial update of encoders
            updateEncoders();

        // Connect to profile macro changes
        if (hotkeyHandler.profileManager) {
            hotkeyHandler.profileManager.macrosChanged.connect(function() {
                console.log("Macros changed, refreshing UI");
                refreshCounter++;
            });
        }

        // Load profile icon for initial profile
        loadProfileIcon();

        // Force initial refresh to load saved icons
        refreshCounter++;
    }

    // Function to update encoder selections based on current profile
    function updateEncoders() {
        if (!hotkeyHandler || !hotkeyHandler.profileManager) return;
        console.log("=== Updating encoders for profile:", hotkeyHandler.profileManager.name);

        // Update Encoder 1
        encoder1Combo.isUpdating = true;
        try {
            const macroData1 = hotkeyHandler.profileManager.getMacroData(-2);
            console.log("Encoder 1 type:", macroData1.type, "content:", macroData1.content);

            if (macroData1.type === "encoder" && macroData1.content !== "") {
                const i1 = encoder1Combo.model.indexOf(macroData1.content);
                console.log("Found encoder 1 content '" + macroData1.content + "' at index:", i1);
                encoder1Combo.currentIndex = i1 >= 0 ? i1 : 0;
            } else {
                console.log("No valid macro for encoder 1, setting to None");
                encoder1Combo.currentIndex = 0;
            }
        } catch (e) {
            console.log("Error updating encoder 1:", e);
            encoder1Combo.currentIndex = 0;
        }
        encoder1Combo.isUpdating = false;

        // Update Encoder 2
        encoder2Combo.isUpdating = true;
        try {
            const macroData2 = hotkeyHandler.profileManager.getMacroData(-1);
            console.log("Encoder 2 type:", macroData2.type, "content:", macroData2.content);

            if (macroData2.type === "encoder" && macroData2.content !== "") {
                const i2 = encoder2Combo.model.indexOf(macroData2.content);
                console.log("Found encoder 2 content '" + macroData2.content + "' at index:", i2);
                encoder2Combo.currentIndex = i2 >= 0 ? i2 : 0;
            } else {
                console.log("No valid macro for encoder 2, setting to None");
                encoder2Combo.currentIndex = 0;
            }
        } catch (e) {
            console.log("Error updating encoder 2:", e);
            encoder2Combo.currentIndex = 0;
        }
        encoder2Combo.isUpdating = false;

        console.log("=== Finished updating encoders ===");
    }

    function loadProfileIcon() {
        var appName = hotkeyHandler.profileManager.getApp();
        console.log("Loading profile icon for app:", appName);

        if (!appName || appName === "") {
            console.log("No app set for this profile");
            return;
        }

        // Check if icon is already saved in profile
        var savedIconPath = hotkeyHandler.profileManager.getMacroImagePath(0);
        console.log("Saved icon path:", savedIconPath);

        if (savedIconPath && savedIconPath !== "") {
            console.log("Profile has saved icon, forcing refresh");
            refreshCounter++;
            return;
        }

        // Try to re-extract icon from saved executable path
        var execPath = hotkeyHandler.profileManager.getAppExecutablePath();
        console.log("Saved executable path:", execPath);

        if (execPath && execPath !== "") {
            try {
                var iconPath = iconExtractor.extractIconForApp(execPath);
                if (iconPath && iconPath !== "") {
                    console.log("Successfully re-extracted icon from saved path");
                    hotkeyHandler.profileManager.setKeyImage(0, "file:///" + iconPath);
                    hotkeyHandler.profileManager.saveProfile();
                    refreshCounter++;
                    return;
                }
            } catch (e) {
                console.log("Failed to extract icon:", e);
            }
        }

        console.log("Could not load icon for app:", appName);
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
                        source: {
                            refreshCounter; // Force refresh when this changes
                            if (!hotkeyHandler || !hotkeyHandler.profileManager) return "";
                                    return hotkeyHandler.profileManager.getMacroImagePath(index + 1);
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
                            var profile = hotkeyHandler.profileManager;
                            var macroData = profile.getMacroData(index + 1);
                            var macro = macroData.type !== "" ? macroData : null;
                            var existingKey = macro ? {
                                keystroke: macroData.type === "keystroke" ? macroData.content : "",
                                executable: macroData.type === "executable" ? macroData.content : ""
                            } : { keystroke: "", executable: "" };

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
        model: hotkeyHandler ? hotkeyHandler.profiles : []
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
            hotkeyHandler.profileManager = hotkeyHandler.profiles[currentIndex];
            exetext.text = hotkeyHandler.profileManager.getApp();
            console.log("Selected profile:", hotkeyHandler.profileManager.name);

            // Reconnect to new profile's macrosChanged signal
            if (hotkeyHandler.profileManager) {
                hotkeyHandler.profileManager.macrosChanged.connect(function() {
                    console.log("Macros changed, refreshing UI");
                    refreshCounter++;
                });
            }

            // Load/re-extract the profile icon if an app is set
            loadProfileIcon();

            // Update encoders when profile changes
            Qt.callLater(root.updateEncoders);

            // Force refresh of all keys
            refreshCounter++;
        }
    }

    // ---------- app selection logic --------
    property string fileDialogCaller: ""

    FileDialog {
        id: fileDialog
        title: "Select an Executable File"
        fileMode: FileDialog.OpenFile
        nameFilters: ["Executable Files (*.exe *.app *.sh)", "All Files (*)"]

        onAccepted: {
            let fullPath = selectedFile.toString().replace("file://", "");
            if(fileDialogCaller === "exebutton"){
                console.log("Selected executable:", selectedFile)

                // extract the app name from the full path so the app tracker can match it to just the name
                let appName = fullPath.split("/").pop().replace(".exe", "").replace(".app", "").replace(".sh", "");

                // makes sure to set the appname AND executable path of the selected profile and save it
                profileManager.setAppWithPath(appName, fullPath);

                // displays the name of the app for the selected profile in the UI
                exetext.text = hotkeyHandler.profileManager.getApp();

                let iconPath = iconExtractor.extractIconForApp(fullPath);
                // Store the icon path in the profile itself, not just in exeIconPath
                if (iconPath !== "") {
                    hotkeyHandler.profileManager.setKeyImage(0, "file:///" + iconPath);
                    hotkeyHandler.profileManager.saveProfile();
                    exeIconPath = "file:///" + iconPath;
                    refreshCounter++;
                } else {
                    exeIconPath = "";
                }
            }
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
        onClicked: {
            fileDialogCaller = "exebutton"
            fileDialog.open()
        }
    }

    // where the name of the selected app for the profile is displayed
    Button {
        id: exetext
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
        id: exeIcon
        width: 40
        height: 40
        anchors.top: exetext.bottom
        anchors.topMargin: 10
        anchors.horizontalCenter: exetext.horizontalCenter
        fillMode: Image.PreserveAspectFit
        source: {
            refreshCounter; // Force refresh
            if (profileSelector.currentText === "General") return "";
            // Get the profile icon from key 0 (reserved for profile icon)
            return hotkeyHandler.profileManager.getMacroImagePath(0);
        }
        visible: profileSelector.currentText !== "General" && source !== ""
    }

    //two encoder knobs
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
                    model: ["None","Scroll","Volume","Chrome Tabs","Switch Apps","Brightness","Zoom","App Volume"]
                    width: 200

                    property bool isUpdating: false

                    onCurrentIndexChanged: {
                        if (isUpdating) {
                            console.log("Encoder 1 - Skipping update (isUpdating=true)");
                            return;
                        }

                        const val = model[currentIndex]
                        console.log("Encoder 1 - User changed to:", val);
                        profileManager.setKeyConfig(-2, "encoder",
                            val !== "None" ? val : "")
                    }
                }

                Button {
                    id: volButton1
                    width: 50
                    height: 20
                    text: "Select Executable"
                    visible: encoder1Combo.currentText === "App Volume"
                    onClicked: {
                        fileDialogCaller = "encoder1"
                        fileDialog.open()
                    }
                }
            }

            Row {
                spacing: 10
                Text { text: "Encoder 2:"; width: 100; color: "white" }

                ComboBox {
                    id: encoder2Combo
                    model: ["None", "Scroll", "Volume", "Chrome Tabs", "Switch Apps", "Brightness", "Zoom", "App Volume"]
                    width: 200

                    property bool isUpdating: false

                    onCurrentIndexChanged: {
                        if (isUpdating) {
                            console.log("Encoder 2 - Skipping update (isUpdating=true)");
                            return;
                        }

                        const val = model[currentIndex]
                        console.log("Encoder 2 - User changed to:", val);
                        profileManager.setKeyConfig(-1, "encoder",
                            val !== "None" ? val : "")
                    }
                }

                Button {
                    id: volButton2
                    width: 50
                    height: 20
                    text: "Select Executable"
                    visible: encoder2Combo.currentText === "App Volume"
                    onClicked: {
                        fileDialogCaller = "encoder2"
                        fileDialog.open()
                    }
                }
            }
        }
    }
}
