declare function print(message: string): void;

declare function DefineOpcode(opcode: number, name: string, parameters: string[], returnType: string): void;
declare function MakeAlias(opcode: number, name: string): void;

declare function SetTextSize(width: number, height: number): void;
declare function SetTextColor(r: number, g: number, b: number, a: number): void;
declare function SetTextCentered(centered: boolean): void;
declare function DrawText(x: number, y: number, text: string): void;
declare function DrawRect(left: number, top: number, right: number, bottom: number, r: number, g: number, b: number, a: number): void;

// MenuSZK
declare function CREATE_SCRIPT_ENTRY(displayText: string, data: string): void;
declare function info(message: string, timeMs: number): void;
declare function ShowBottomMessage(message: string, duration: number): void;
declare function OpenKeyboard(initialInput: string, flags: KeyboardFlags): void;