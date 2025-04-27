import QtQuick
import QtQuick.Controls
import QtCore 6.2
import FileIO 1.0
import Macro 1.0

Item {
    id: profileManager

    property var profiles: mainWindow.profiles

    function setKeyConfig(keyIndex, type, value) {
        console.log("Updating key:", keyIndex, "Type:", type, "Value:", value);

        if (type === "keystroke" && value !== "") {
            mainWindow.profileInstance.setMacro(keyIndex, "keystroke", value);
        } else if (type === "executable" && value !== "") {
            mainWindow.profileInstance.setMacro(keyIndex, "executable", value);
        }

        mainWindow.profileInstance.saveProfile();
    }
}
