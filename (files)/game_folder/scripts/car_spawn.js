export { };

// ---------------------------------------------------------
// Vehicle configuration
// ---------------------------------------------------------

const CAR_MIN_MODEL = 400;
const CAR_MAX_MODEL = 611;

const vehicleIds = [
    596,
    597,
    598,
    599,
    523,
    470
];

const INVALID_CAR_MODELS = [
    441,
    449,
    464,
    465,
    501,
    537,
    538,
    564,
    569,
    570,
    577,
    590,
    592,
    594
];

let waitingForKeyboard = false;

// ---------------------------------------------------------
// Initialization
// ---------------------------------------------------------

function onRegisterAliases() {
    MakeAlias(0x01F5, "GET_PLAYER_ACTOR");
}

function onLoaded() {
    CREATE_SCRIPT_ENTRY(
        "Vehicle Spawner (PMESP)",
        "open_vehicle_spawner_pmesp"
    );

    CREATE_SCRIPT_ENTRY(
        "Vehicle Spawner (All)",
        "open_vehicle_spawner_all"
    );

    CREATE_SCRIPT_ENTRY(
        "Vehicle spawn (keyboard)",
        "open_vehicle_spawner_by_id"
    );
}

/**
 * @param {string} data
 */
function onScriptCalled(data) {
    if (data == "open_vehicle_spawner_pmesp") {
        OpenCarSpawner(true);
        return;
    }

    if (data == "open_vehicle_spawner_all") {
        OpenCarSpawner(false);
        return;
    }

    if (data == "open_vehicle_spawner_by_id") {
        waitingForKeyboard = true;

        const KeyboardFlags = {
            None: 0,
            NumbersOnly: 1 << 0
        };

        OpenKeyboard("", KeyboardFlags.NumbersOnly);

        return;
    }
}

/**
 * @param {string} input
 */
function onKeyboardEnterPressed(input) {
    if (!waitingForKeyboard) {
        return;
    }

    waitingForKeyboard = false;

    SpawnCarById(input);
}

// ---------------------------------------------------------
// Drawing
// ---------------------------------------------------------

function onDraw() {
    if (carSpawnerActive) {
        UpdateCarSpawnerText();
    }
}


// ---------------------------------------------------------
// Input helpers
// ---------------------------------------------------------

/**
 * @param {number} point
 * @param {number} minTime
 */
function GetTouchPointState(point, minTime) {
    return GET_TOUCH_POINT_STATE(point, minTime);
}

function GetMenuButtonState() {
    return GET_MENU_BUTTON_STATE();
}


// ---------------------------------------------------------
// Input configuration
// ---------------------------------------------------------

const CAR_PREV_TOUCH_POINT = 2;
const CAR_NEXT_TOUCH_POINT = 8;
const CAR_SELECT_TOUCH_POINT = 5;
const CAR_CANCEL_TOUCH_POINT = 9;

const CAR_CHANGE_INTERVAL = 250;


// ---------------------------------------------------------
// State
// ---------------------------------------------------------

let useVehicleList = false;

let vehicleIdIndex = 0;

let carModel = CAR_MIN_MODEL;

let previewCar = 0;

let carSpawnerActive = false;

let loadingCarModel = false;

let carPreviewDelay = 0;

let lastCarChangeTime = 0;


// ---------------------------------------------------------
// Model validation
// ---------------------------------------------------------

/**
 * @param {number} model
 */
function IsInvalidCarModel(model) {
    for (let i = 0; i < INVALID_CAR_MODELS.length; i++) {
        if (INVALID_CAR_MODELS[i] === model) {
            return true;
        }
    }

    return false;
}


// ---------------------------------------------------------
// Model selection
// ---------------------------------------------------------

function SetInitialCarModel() {
    if (useVehicleList) {
        if (vehicleIds.length === 0) {
            carModel = CAR_MIN_MODEL;
            return;
        }

        vehicleIdIndex = 0;
        carModel = vehicleIds[vehicleIdIndex];

        return;
    }

    carModel = CAR_MIN_MODEL;

    while (IsInvalidCarModel(carModel)) {
        carModel++;

        if (carModel > CAR_MAX_MODEL) {
            carModel = CAR_MIN_MODEL;
        }
    }
}

function GetNextCarModel() {
    if (useVehicleList) {
        if (vehicleIds.length === 0) {
            return;
        }

        vehicleIdIndex++;

        if (vehicleIdIndex >= vehicleIds.length) {
            vehicleIdIndex = 0;
        }

        carModel = vehicleIds[vehicleIdIndex];

        return;
    }

    do {
        carModel++;

        if (carModel > CAR_MAX_MODEL) {
            carModel = CAR_MIN_MODEL;
        }
    }
    while (IsInvalidCarModel(carModel));
}

function GetPreviousCarModel() {
    if (useVehicleList) {
        if (vehicleIds.length === 0) {
            return;
        }

        vehicleIdIndex--;

        if (vehicleIdIndex < 0) {
            vehicleIdIndex = vehicleIds.length - 1;
        }

        carModel = vehicleIds[vehicleIdIndex];

        return;
    }

    do {
        carModel--;

        if (carModel < CAR_MIN_MODEL) {
            carModel = CAR_MAX_MODEL;
        }
    }
    while (IsInvalidCarModel(carModel));
}


// ---------------------------------------------------------
// Preview
// ---------------------------------------------------------

function RequestCarPreview() {
    if (loadingCarModel) {
        return;
    }

    if (previewCar) {
        DELETE_CAR(previewCar);
        previewCar = 0;
    }

    REQUEST_MODEL(carModel);
    LOAD_ALL_MODELS_NOW();

    loadingCarModel = true;
}

function CreateCarPreview() {
    if (!loadingCarModel) {
        return;
    }

    if (!HAS_MODEL_LOADED(carModel)) {
        return;
    }

    loadingCarModel = false;

    const playerActor = GET_PLAYER_ACTOR(0);

    if (!playerActor) {
        return;
    }

    const dimensions = GET_MODEL_DIMENSIONS(carModel);

    let offsetY = dimensions[4];

    if (IS_THIS_MODEL_A_PLANE(carModel)) {
        offsetY *= 1.2;
    }
    else {
        offsetY *= 2.2;
        offsetY += 1.0;
    }

    const position = GET_OFFSET_FROM_CHAR_IN_WORLD_COORDS(
        playerActor,
        0.2,
        offsetY,
        0.0
    );

    let heading = GET_CHAR_HEADING(playerActor);

    if (heading > 180.0) {
        heading -= 180.0;
    }
    else {
        heading += 180.0;
    }

    const car = CREATE_CAR(
        carModel,
        position[0],
        position[1],
        position[2]
    );

    if (!car) {
        MARK_MODEL_AS_NO_LONGER_NEEDED(carModel);
        return;
    }

    heading -= 90.0;

    SET_CAR_HEADING(car, heading);
    LOCK_CAR_DOORS(car, 2);

    previewCar = car;

    MARK_MODEL_AS_NO_LONGER_NEEDED(carModel);
}

/**
 * @param {string} input
 */
function SpawnCarById(input) {
    const model = parseInt(input, 10);

    if (isNaN(model)) {
        ShowBottomMessage("ID inválido", 3000);
        return;
    }

    if (model < CAR_MIN_MODEL || model > CAR_MAX_MODEL) {
        ShowBottomMessage("ID de veículo inválido: " + model, 3000);
        return;
    }

    if (IsInvalidCarModel(model)) {
        ShowBottomMessage("Esse ID não é um veículo válido: " + model, 3000);
        return;
    }

    const playerActor = GET_PLAYER_ACTOR(0);

    if (!playerActor) {
        return;
    }

    REQUEST_MODEL(model);
    LOAD_ALL_MODELS_NOW();

    if (!HAS_MODEL_LOADED(model)) {
        ShowBottomMessage("Não foi possível carregar o veículo " + model, 3000);
        return;
    }

    const dimensions = GET_MODEL_DIMENSIONS(model);

    let offsetY = dimensions[4];

    if (IS_THIS_MODEL_A_PLANE(model)) {
        offsetY *= 1.2;
    }
    else {
        offsetY *= 2.2;
        offsetY += 1.0;
    }

    const position = GET_OFFSET_FROM_CHAR_IN_WORLD_COORDS(
        playerActor,
        0.2,
        offsetY,
        0.0
    );

    let heading = GET_CHAR_HEADING(playerActor);

    if (heading > 180.0) {
        heading -= 180.0;
    }
    else {
        heading += 180.0;
    }

    const car = CREATE_CAR(
        model,
        position[0],
        position[1],
        position[2]
    );

    if (!car) {
        MARK_MODEL_AS_NO_LONGER_NEEDED(model);
        ShowBottomMessage("Falha ao criar o veículo", 3000);
        return;
    }

    heading -= 90.0;

    SET_CAR_HEADING(car, heading);

    MARK_MODEL_AS_NO_LONGER_NEEDED(model);

    LOCK_CAR_DOORS(car, 0);
    MARK_CAR_AS_NO_LONGER_NEEDED(car);

    ShowBottomMessage("Vehicle spawned: ID " + model, 3000);
}

// ---------------------------------------------------------
// Text
// ---------------------------------------------------------

function UpdateCarSpawnerText() {
    SetTextSize(4.0, 4.0);
    SetTextColor(255, 0, 0, 255);
    SetTextCentered(true);

    DrawText(40.0, 220.0, "LEFT");
    DrawText(600.0, 220.0, "RIGHT");

    SetTextSize(3.0, 3.0);
    SetTextColor(255, 255, 255, 255);

    DrawText(320.0, 300.0, "ID: " + carModel);
    DrawText(320.0, 360.0, "Selecionar");

    DrawText(560.0, 400.0, "Cancelar");
}


// ---------------------------------------------------------
// Open / Close
// ---------------------------------------------------------

/**
 * @param {boolean} useList
 */
function OpenCarSpawner(useList) {
    print("opening car");

    useVehicleList = useList;

    SetInitialCarModel();

    carSpawnerActive = true;
    loadingCarModel = false;
    previewCar = 0;
    carPreviewDelay = 500;
    lastCarChangeTime = 0;

    const playerActor = GET_PLAYER_ACTOR(0);

    SET_PLAYER_CONTROL(0, false);

    FREEZE_CHAR_POSITION_AND_DONT_LOAD_COLLISION(
        playerActor,
        true
    );

    RESTORE_CAMERA_JUMPCUT();
    SET_CAMERA_BEHIND_PLAYER();
}

function CancelCarSpawner() {
    if (previewCar) {
        DELETE_CAR(previewCar);
        previewCar = 0;
    }

    if (loadingCarModel) {
        MARK_MODEL_AS_NO_LONGER_NEEDED(carModel);
        loadingCarModel = false;
    }

    SET_PLAYER_CONTROL(0, true);

    FREEZE_CHAR_POSITION_AND_DONT_LOAD_COLLISION(
        GET_PLAYER_ACTOR(0),
        false
    );

    carSpawnerActive = false;
    carPreviewDelay = 0;
}

function SelectCar() {
    if (!previewCar) {
        return;
    }

    LOCK_CAR_DOORS(previewCar, 0);
    MARK_CAR_AS_NO_LONGER_NEEDED(previewCar);

    previewCar = 0;

    SET_PLAYER_CONTROL(0, true);

    FREEZE_CHAR_POSITION_AND_DONT_LOAD_COLLISION(
        GET_PLAYER_ACTOR(0),
        false
    );

    carSpawnerActive = false;
    carPreviewDelay = 0;
}


// ---------------------------------------------------------
// Input
// ---------------------------------------------------------

function IsSelectPressed() {
    return GetTouchPointState(
        CAR_SELECT_TOUCH_POINT,
        0
    );
}

function IsCancelPressed() {
    if (GetTouchPointState(CAR_CANCEL_TOUCH_POINT, 0)) {
        return true;
    }

    if (GetTouchPointState(1, 500)) {
        return true;
    }

    if (GetTouchPointState(4, 500)) {
        return true;
    }

    if (GetTouchPointState(7, 500)) {
        return true;
    }

    if (GetTouchPointState(3, 250)) {
        return true;
    }

    return false;
}

function CanChangeCarModel() {
    const now = Date.now();

    return now - lastCarChangeTime >= CAR_CHANGE_INTERVAL;
}

function UpdateCarSelection() {
    if (!CanChangeCarModel()) {
        return false;
    }

    if (GetTouchPointState(CAR_NEXT_TOUCH_POINT, 0)) {
        GetNextCarModel();

        lastCarChangeTime = Date.now();

        RequestCarPreview();

        return true;
    }

    if (GetTouchPointState(CAR_PREV_TOUCH_POINT, 0)) {
        GetPreviousCarModel();

        lastCarChangeTime = Date.now();

        RequestCarPreview();

        return true;
    }

    return false;
}


// ---------------------------------------------------------
// Main update
// ---------------------------------------------------------

function UpdateCarSpawner() {
    if (!carSpawnerActive) {
        return;
    }

    // Initial delay, equivalent to the original wait 500.
    if (carPreviewDelay > 0) {
        carPreviewDelay -= 16;

        if (carPreviewDelay > 0) {
            return;
        }
    }

    // Start loading the first/next vehicle.
    if (!loadingCarModel && !previewCar) {
        RequestCarPreview();
        return;
    }

    // Wait for the model without blocking the game.
    if (loadingCarModel) {
        CreateCarPreview();
        return;
    }

    if (IsCancelPressed()) {
        CancelCarSpawner();
        return;
    }

    if (IsSelectPressed()) {
        SelectCar();
        return;
    }

    UpdateCarSelection();
}


// ---------------------------------------------------------
// Game process
// ---------------------------------------------------------

function onGameProcess() {
    UpdateCarSpawner();
}


// ---------------------------------------------------------
// State
// ---------------------------------------------------------

let carSpawnerInitialized = false;
let carSpawnerOpened = false;

let lastTouchLogTime = 0;