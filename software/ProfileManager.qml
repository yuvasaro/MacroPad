import QtQuick
import QtQuick.Controls
import QtCore 6.2
import FileIO 1.0
import Macro 1.0

Item {
    id: profileManager
    property var profiles: hotkeyHandler.profiles

    function setKeyConfig(keyIndex, type, value) {
        console.log("Updating key:", keyIndex, "Type:", type, "Value:", value);

        if (type === "keystroke" && value !== "") {
            hotkeyHandler.profileManager.setMacro(keyIndex, "keystroke", value);
        } else if (type === "executable" && value !== "") {
            hotkeyHandler.profileManager.setMacro(keyIndex, "executable", value);
        } else if(type === "encoder" && value !== "None" && value !=="" && !(value === undefined)){
            console.log("[setMacro to]",value);
            hotkeyHandler.profileManager.setMacro(keyIndex, "encoder", value);
        }

        hotkeyHandler.profileManager.saveProfile();
        console.log("[save profile to]",value);
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
