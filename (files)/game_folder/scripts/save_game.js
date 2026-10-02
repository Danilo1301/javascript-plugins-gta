function onLoaded() {
    CREATE_SCRIPT_ENTRY("Save game", "save_my_game");
}

/**
 * @param {string} data
 */
function onScriptCalled(data) {

    if (data == "save_my_game") {

        info("Saving...", 10000);

        ACTIVATE_SAVE_MENU();
    }
}