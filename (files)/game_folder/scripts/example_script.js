//export { };

let messageVisible = true;
let carId = -1;
let carModel = -1;

function onRegisterOpcodes() {

    // Define custom opcodes using Sanny Builder types.
    // DefineOpcode(0x01F5, "GET_PLAYER_CHAR", ["int", "int*"], "void");
}

function onRegisterAliases() {

    // Create a readable name for an opcode.
    MakeAlias(0x01F5, "GET_PLAYER_ACTOR");
}

function onLoaded() {

    // Add an entry to the script menu.
    CREATE_SCRIPT_ENTRY("Run example script", "run_example_script1");
}

/**
 * @param {string} data
 */
function onScriptCalled(data) {

    // Called when a script entry is selected.
    if (data == "run_example_script1") {
        runThisScriptOnce();
    }
}

function onDraw() {

    if (!messageVisible) return;

    const BASE_WIDTH = 640;
    const BASE_HEIGHT = 448;

    const centerX = BASE_WIDTH / 2;
    const centerY = BASE_HEIGHT / 2;

    SetTextSize(3.0, 3.0);
    SetTextColor(255, 255, 255, 255);
    SetTextCentered(true);

    const rectWidth = 280.0;
    const rectHeight = 60.0;

    DrawRect(
        centerX - rectWidth / 2,
        centerY - rectHeight / 2,
        centerX + rectWidth / 2,
        centerY + rectHeight / 2,
        255, 0, 0, 120);

    DrawText(centerX, centerY, "Example script working");

    const carText = carId >= 0 ? `${carId} (${carModel})` : "NONE";

    SetTextSize(2.0, 2.0);
    DrawText(centerX, centerY + 40, "Your car ID: " + carText);

}

function runThisScriptOnce() {

    messageVisible = true;

    info("running example script", 2000);

    // Call GTA opcodes directly.
    const defined = IS_PLAYER_PLAYING(0);

    if (!defined) {
        print("Player 0 is not playing");
        return;
    }

    const playerActor = GET_PLAYER_ACTOR(0);

    if (playerActor <= 0) {
        print("Invalid player actor");
        return;
    }

    const coords = GET_CHAR_COORDINATES(playerActor);

    print("X = " + coords[0]);
    print("Y = " + coords[1]);
    print("Z = " + coords[2]);

    const car = GET_CAR_CHAR_IS_USING(playerActor);

    print("CAR = " + car);

    carId = car;
    carModel = carId > 0 ? GET_CAR_MODEL(carId) : -1;

    ShowBottomMessage("example script finalized", 3000);
}

function onGameProcess() {

    // Called every game update.

    if (messageVisible) {
        processHideMessage();
    }
}

function onTimerUpdate() {

    // Called every CTimer update.

    if (messageVisible) {
        processHideMessage();
    }
}

function processHideMessage() {
    for (let point = 1; point <= 9; point++) {
        const result = GET_TOUCH_POINT_STATE(point, 0);

        if (result) {
            messageVisible = false;
            return;
        }
    }
}