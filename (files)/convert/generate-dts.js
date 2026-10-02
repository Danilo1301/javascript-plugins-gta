const fs = require("fs");
const path = require("path");

const inputPath = process.argv[2];

if (!inputPath) {
    console.error("Usage: node generate-dts.js <input.js> [output.d.ts]");
    process.exit(1);
}

const outputPath = process.argv[3] || path.join(
    path.dirname(inputPath),
    path.basename(inputPath, path.extname(inputPath)) + ".d.ts"
);

const source = fs.readFileSync(inputPath, "utf8");

const TYPE_MAP = {
    int: "number",
    float: "number",
    double: "number",
    bool: "boolean",
    string: "string",
    void: "void"
};

const POINTER_TYPE_MAP = {
    "int*": "number",
    "float*": "number",
    "bool*": "boolean"
};

function parseArguments(text) {
    if (!text.trim()) {
        return [];
    }

    return text
        .split(",")
        .map(value => value.trim())
        .map(value => value.replace(/^["']|["']$/g, ""));
}

function getType(type) {
    return TYPE_MAP[type] || "unknown";
}

function getPointerType(type) {
    return POINTER_TYPE_MAP[type] || "unknown";
}

function generateReturnType(argumentsList, returnType) {
    if (returnType !== "void") {
        return getType(returnType);
    }

    const outputs = argumentsList
        .filter(type => type in POINTER_TYPE_MAP)
        .map(type => getPointerType(type));

    if (outputs.length === 0) {
        return "void";
    }

    if (outputs.length === 1) {
        return outputs[0];
    }

    return `[${outputs.join(", ")}]`;
}

function generateFunction(name, argumentsList, returnType) {
    const jsArguments = argumentsList
        .filter(type => !(type in POINTER_TYPE_MAP));

    const parameters = jsArguments
        .map((type, index) => {
            return `arg${index}: ${getType(type)}`;
        })
        .join(", ");

    const resultType = generateReturnType(argumentsList, returnType);

    return `declare function ${name}(${parameters}): ${resultType};`;
}

const defineOpcodeRegex =
    /DefineOpcode\(\s*(0x[0-9A-Fa-f]+|\d+)\s*,\s*"([^"]+)"\s*,\s*\[([^\]]*)\]\s*,\s*"([^"]+)"\s*\)/g;

const opcodes = [];

for (const match of source.matchAll(defineOpcodeRegex)) {
    const opcode = Number(match[1]);
    const name = match[2];
    const argumentsList = parseArguments(match[3]);
    const returnType = match[4];

    opcodes.push({
        opcode,
        name,
        arguments: argumentsList,
        returnType
    });
}

opcodes.sort((a, b) => a.opcode - b.opcode);

const lines = [
    "// Automatically generated. Do not edit.",
    "",
    ...opcodes.map(opcode => {
        return generateFunction(
            opcode.name,
            opcode.arguments,
            opcode.returnType
        );
    }),
    ""
];

fs.writeFileSync(outputPath, lines.join("\n"), "utf8");

console.log(`Generated ${opcodes.length} opcode declarations.`);
console.log(`Output: ${outputPath}`);