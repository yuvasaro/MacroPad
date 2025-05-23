import QtQuick
import QtQuick.Controls
import QtCore 6.2
import IconExtractor 1.0


Item {
    id: profileManager
    property var profiles: hotkeyHandler.profiles

    IconExtractor {
        id: iconExtractor
    }

    function setKeyConfig(keyIndex, type, value, customImage) {
        console.log("Updating key:", keyIndex, "Type:", type, "Value:", value, "CustomImage:", customImage);
        let image = "";

        if (customImage && customImage !== "") {
            // Use provided custom image
            image = customImage;
        } else if (type === "keystroke" && value !== "") {
            // Default keystroke icon
            image = "qrc:/keystroke.jpg";
        } else if (type === "executable" && value !== "") {
        try {
            let extractedPath = iconExtractor.extractIconForApp(value);
            if (extractedPath && extractedPath !== "") {
                image = "file://" + extractedPath;
            } else {
                image = "qrc:/executable.png";  // fallback
            }
        } catch (e) {
            console.log("Couldn't extract icon:", e);
            image = "qrc:/executable.png";
        }
    }

        if (type === "image") {
            hotkeyHandler.profileManager.setKeyImage(keyIndex, value);
        } else if (type === "keystroke" && value !== "") {
            hotkeyHandler.profileManager.setMacro(keyIndex, "keystroke", value);
            hotkeyHandler.profileManager.setKeyImage(keyIndex, image);
        } else if (type === "executable" && value !== "") {
            hotkeyHandler.profileManager.setMacro(keyIndex, "executable", value);
            hotkeyHandler.profileManager.setKeyImage(keyIndex, image);
        }

        hotkeyHandler.profileManager.saveProfile();
    }

    function setApp(app) {
        console.log("app changed");
        hotkeyHandler.profileManager.setApp(app);
        hotkeyHandler.profileManager.saveProfile();
    }

    Component.onCompleted: {
        hotkeyHandler.profileManager.saveProfile();
    }
}
