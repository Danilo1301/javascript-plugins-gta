export { };

// ---------------------------------------------------------
// Skin configuration
// ---------------------------------------------------------

const SKIN_SELECTION_DELAY = 100;
const SKIN_APPLY_DELAY = 500;

const skinIds = [
    282,
    281,
    280,
    279,
    278,
    277,
    276,
    275,
    274,

    283,
    284,
    285,
    286,
    287,
    288
];

let waitingForKeyboard = false;


// ---------------------------------------------------------
// Initialization
// ---------------------------------------------------------

function onLoaded() {
    CREATE_SCRIPT_ENTRY(
        "Skins (PMESP)",
        "open_skin_changer_pmesp"
    );
}


// ---------------------------------------------------------
// Script events
// ---------------------------------------------------------

/**
 * @param {string} data
 */
function onScriptCalled(data) {
    if (data == "open_skin_changer_pmesp") {
        OpenSkinChanger();
        return;
    }
}

// ---------------------------------------------------------
// Drawing
// ---------------------------------------------------------

function onDraw() {
    if (skinChangerActive) {
        UpdateSkinChangerText();
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

const SKIN_PREV_TOUCH_POINT = 2;
const SKIN_NEXT_TOUCH_POINT = 8;
const SKIN_SELECT_TOUCH_POINT = 5;
const SKIN_CANCEL_TOUCH_POINT = 9;


// ---------------------------------------------------------
// State
// ---------------------------------------------------------

let skinIdIndex = 0;
let skinModel = skinIds[0];

let skinChangerActive = false;

let loadingSkinModel = false;
let loadingSkinModelId = -1;

let appliedSkinModel = -1;

let skinPreviewReady = false;

let skinPreviewDelay = 0;

let lastSkinSelectionTime = 0;
let lastSkinApplyTime = 0;


// ---------------------------------------------------------
// Model selection
// ---------------------------------------------------------

function SetInitialSkinModel() {
    if (skinIds.length === 0) {
        skinIdIndex = 0;
        skinModel = 0;
        return;
    }

    if (skinIdIndex < 0 || skinIdIndex >= skinIds.length) {
        skinIdIndex = 0;
    }

    skinModel = skinIds[skinIdIndex];
}

function GetNextSkinModel() {
    if (skinIds.length === 0) {
        return;
    }

    skinIdIndex++;

    if (skinIdIndex >= skinIds.length) {
        skinIdIndex = 0;
    }

    skinModel = skinIds[skinIdIndex];

    skinPreviewReady = false;
}

function GetPreviousSkinModel() {
    if (skinIds.length === 0) {
        return;
    }

    skinIdIndex--;

    if (skinIdIndex < 0) {
        skinIdIndex = skinIds.length - 1;
    }

    skinModel = skinIds[skinIdIndex];

    skinPreviewReady = false;
}


// ---------------------------------------------------------
// Preview
// ---------------------------------------------------------

function RequestSkinPreview() {
    if (!skinChangerActive) {
        return;
    }

    if (loadingSkinModel) {
        return;
    }

    if (skinModel === appliedSkinModel) {
        skinPreviewReady = true;
        return;
    }

    loadingSkinModelId = skinModel;
    loadingSkinModel = true;

    print(
        "Requesting model " +
        loadingSkinModelId
    );

    REQUEST_MODEL(
        loadingSkinModelId
    );
}

function CreateSkinPreview() {
    if (!loadingSkinModel) {
        return false;
    }

    if (!HAS_MODEL_LOADED(loadingSkinModelId)) {
        return false;
    }

    const modelToApply = loadingSkinModelId;

    print(
        "Model loaded " +
        modelToApply
    );

    SET_PLAYER_MODEL(
        0,
        modelToApply
    );

    appliedSkinModel = modelToApply;

    loadingSkinModel = false;
    loadingSkinModelId = -1;

    skinPreviewReady = true;

    lastSkinApplyTime = Date.now();

    return true;
}


// ---------------------------------------------------------
// Preview update
// ---------------------------------------------------------

function UpdateSkinPreview() {
    const now = Date.now();

    if (now - lastSkinApplyTime < SKIN_APPLY_DELAY) {
        return;
    }

    if (loadingSkinModel) {
        CreateSkinPreview();
        return;
    }

    if (skinModel === appliedSkinModel) {
        skinPreviewReady = true;
        return;
    }

    RequestSkinPreview();
}


// ---------------------------------------------------------
// Change player skin
// ---------------------------------------------------------

/**
 * @param {number} model
 */
function ChangePlayerSkin(model) {
    const playerActor = GET_PLAYER_ACTOR(0);

    if (!playerActor) {
        return false;
    }

    REQUEST_MODEL(model);

    LOAD_ALL_MODELS_NOW();

    if (!HAS_MODEL_LOADED(model)) {
        return false;
    }

    SET_PLAYER_MODEL(
        0,
        model
    );

    appliedSkinModel = model;
    lastSkinApplyTime = Date.now();

    return true;
}

/**
 * @param {string} input
 */
function ChangePlayerSkinById(input) {
    const model = parseInt(input, 10);

    if (isNaN(model)) {
        ShowBottomMessage(
            "ID inválido",
            3000
        );

        return;
    }

    const index = skinIds.indexOf(model);

    if (index === -1) {
        ShowBottomMessage(
            "Essa skin não é de PM: " + model,
            3000
        );

        return;
    }

    if (!ChangePlayerSkin(model)) {
        ShowBottomMessage(
            "Não foi possível carregar a skin " + model,
            3000
        );

        return;
    }

    skinIdIndex = index;
    skinModel = model;

    ShowBottomMessage(
        "Skin alterada: ID " + model,
        3000
    );
}


// ---------------------------------------------------------
// Text
// ---------------------------------------------------------

function UpdateSkinChangerText() {
    SetTextSize(4.0, 4.0);

    SetTextColor(
        255,
        0,
        0,
        255
    );

    SetTextCentered(true);

    DrawText(
        40.0,
        220.0,
        "LEFT"
    );

    DrawText(
        600.0,
        220.0,
        "RIGHT"
    );

    SetTextSize(3.0, 3.0);

    SetTextColor(
        255,
        255,
        255,
        255
    );

    DrawText(
        320.0,
        300.0,
        "SKIN ID: " + skinModel
    );

    DrawText(
        320.0,
        360.0,
        "Selecionar"
    );

    DrawText(
        560.0,
        400.0,
        "Cancelar"
    );
}


// ---------------------------------------------------------
// Open / Close
// ---------------------------------------------------------

function OpenSkinChanger() {
    print("opening skin changer");

    SetInitialSkinModel();

    skinChangerActive = true;

    loadingSkinModel = false;
    loadingSkinModelId = -1;

    skinPreviewReady = false;

    skinPreviewDelay = SKIN_SELECTION_DELAY;

    lastSkinSelectionTime = 0;

    if (appliedSkinModel == -1) {
        lastSkinApplyTime = 0;
    }

    const playerActor = GET_PLAYER_ACTOR(0);

    if (!playerActor) {
        skinChangerActive = false;
        return;
    }

    SET_PLAYER_CONTROL(
        0,
        false
    );

    FREEZE_CHAR_POSITION_AND_DONT_LOAD_COLLISION(
        playerActor,
        true
    );

    RESTORE_CAMERA_JUMPCUT();
    SET_CAMERA_BEHIND_PLAYER();
}

function CancelSkinChanger() {
    loadingSkinModel = false;
    loadingSkinModelId = -1;

    skinPreviewReady = false;

    SET_PLAYER_CONTROL(
        0,
        true
    );

    const playerActor = GET_PLAYER_ACTOR(0);

    if (playerActor) {
        FREEZE_CHAR_POSITION_AND_DONT_LOAD_COLLISION(
            playerActor,
            false
        );
    }

    skinChangerActive = false;
    skinPreviewDelay = 0;
}

function SelectSkin() {
    if (loadingSkinModel) {
        return;
    }

    if (!skinPreviewReady) {
        return;
    }

    SET_PLAYER_CONTROL(
        0,
        true
    );

    const playerActor = GET_PLAYER_ACTOR(0);

    if (playerActor) {
        FREEZE_CHAR_POSITION_AND_DONT_LOAD_COLLISION(
            playerActor,
            false
        );
    }

    skinChangerActive = false;
    skinPreviewReady = false;
    skinPreviewDelay = 0;

    ShowBottomMessage(
        "Skin aplicada: ID " + skinModel,
        3000
    );
}


// ---------------------------------------------------------
// Input
// ---------------------------------------------------------

function IsSelectPressed() {
    return GetTouchPointState(
        SKIN_SELECT_TOUCH_POINT,
        0
    );
}

function IsCancelPressed() {
    if (GetTouchPointState(SKIN_CANCEL_TOUCH_POINT, 0)) {
        return true;
    }

    if (GetTouchPointState(1, SKIN_SELECTION_DELAY)) {
        return true;
    }

    if (GetTouchPointState(4, SKIN_SELECTION_DELAY)) {
        return true;
    }

    if (GetTouchPointState(7, SKIN_SELECTION_DELAY)) {
        return true;
    }

    if (GetTouchPointState(3, SKIN_SELECTION_DELAY)) {
        return true;
    }

    return false;
}

function CanChangeSkinModel() {
    const now = Date.now();

    return now - lastSkinSelectionTime >= SKIN_SELECTION_DELAY;
}

function UpdateSkinSelection() {
    if (!CanChangeSkinModel()) {
        return false;
    }

    if (GetTouchPointState(SKIN_NEXT_TOUCH_POINT, 0)) {
        GetNextSkinModel();

        lastSkinSelectionTime = Date.now();

        return true;
    }

    if (GetTouchPointState(SKIN_PREV_TOUCH_POINT, 0)) {
        GetPreviousSkinModel();

        lastSkinSelectionTime = Date.now();

        return true;
    }

    return false;
}


// ---------------------------------------------------------
// Main update
// ---------------------------------------------------------

function UpdateSkinChanger() {
    if (!skinChangerActive) {
        return;
    }

    if (skinPreviewDelay > 0) {
        skinPreviewDelay -= 16;

        if (skinPreviewDelay > 0) {
            return;
        }
    }

    // Navegação: pode mudar a cada 100ms.
    UpdateSkinSelection();

    // Preview: SET_PLAYER_MODEL só pode acontecer a cada 800ms.
    UpdateSkinPreview();

    if (IsCancelPressed()) {
        CancelSkinChanger();
        return;
    }

    if (IsSelectPressed()) {
        SelectSkin();
        return;
    }
}


// ---------------------------------------------------------
// Game process
// ---------------------------------------------------------

function onGameProcess() {
    UpdateSkinChanger();
}