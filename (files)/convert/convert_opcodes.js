const fs = require("fs");

const inputPath = process.argv[2];

const outputPath = process.argv[3] || "opcodes.js";

if (!inputPath) {
    console.error("Usage: node convert_opcodes.js <input.json> [output.js]");
    process.exit(1);
}

const data = JSON.parse(fs.readFileSync(inputPath, "utf8"));

const handleTypes = new Set([
    "Car",
    "Char",
    "Object",
    "Player",
    "Plane",
    "Heli",
    "Boat",
    "Train",
    "Trailer",
    "Pickup",
    "Particle",
    "ScriptFire",
    "Blip",
    "Group",
    "Searchlight",
    "Checkpoint",
    "Attractor",
    "Event",
    "CarGenerator",
    "DecisionMakerChar",
    "DecisionMakerGroup",
    "TaskId",
    "PedBoneId",
    "HoldOrientation",
    "DriveByType",
    "Menu",
    "WidgetId",
    "model_object",
    "model_vehicle",
    "model_char",
    "gxt_key"
]);

const specialTypes = new Set([
    "label",
    "arguments",
    "script_id",
    "string128"
]);

function normalizeName(name) {
    return name
        .replace(/[^a-zA-Z0-9]+/g, "_")
        .replace(/^_+|_+$/g, "")
        .toUpperCase();
}

function parseType(type) {
    type = type.trim();

    if (type === "int")
        return "int";

    if (type === "float")
        return "float";

    if (type === "bool")
        return "bool";

    if (type === "string")
        return "string";

    if (handleTypes.has(type))
        return "int";

    if (specialTypes.has(type))
        return "int";

    return "int";
}

function convertCommand(command) {
    if (command.attrs?.is_unsupported)
        return null;

    if (!command.id || !command.name)
        return null;

    const opcode = parseInt(command.id, 16);

    if (Number.isNaN(opcode))
        return null;

    const argumentsList = [];

    for (const input of command.input || []) {
        argumentsList.push(parseType(input.type));
    }

    for (const output of command.output || []) {
        argumentsList.push(parseType(output.type) + "*");
    }

    const returnType = command.attrs?.is_condition
        ? "bool"
        : "void";

    return {
        opcode,
        name: normalizeName(command.name),
        arguments: argumentsList,
        returnType
    };
}

const definitions = [];

for (const extension of data.extensions || []) {
    for (const command of extension.commands || []) {
        const definition = convertCommand(command);

        if (!definition)
            continue;

        definitions.push(definition);
    }
}

definitions.sort((a, b) => a.opcode - b.opcode);

const output = [];

output.push("// Generated automatically.");
output.push("");

for (const definition of definitions) {
    const opcode = `0x${definition.opcode
        .toString(16)
        .toUpperCase()
        .padStart(4, "0")}`;

    const argumentsText = definition.arguments
        .map(type => `"${type}"`)
        .join(", ");

    output.push(
        `DefineOpcode(${opcode}, "${definition.name}", [${argumentsText}], "${definition.returnType}");`
    );
}

output.push("");

fs.writeFileSync(outputPath, output.join("\n"), "utf8");

console.log(`Generated ${definitions.length} opcodes.`);
console.log(`Output: ${outputPath}`);