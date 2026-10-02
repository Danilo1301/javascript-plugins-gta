export { };

function onRegisterOpcodes() {
    // MenuSZK
    DefineOpcode(0xF000, "CREATE_SCRIPT_ENTRY", ["string", "string"], "void");
}