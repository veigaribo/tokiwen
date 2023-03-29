import { setup } from "./language/setup";
import { Cpu } from "./vm/cpu";
import { Memory } from "./vm/memory";
import { MemTable } from "./ui/mem-table";
import { InstructionTable } from "./ui/instruction-table";
import { RegisterTable } from "./ui/register-table";
import { VariableTable } from "./ui/variable-table";
import { Input, Output } from "./vm/io";
import { LineIndicator } from "./ui/line-indicator";

const settings = document.getElementById("settings") as HTMLInputElement;
const textarea = document.getElementById("code-textarea") as HTMLInputElement;

const settingsBtn = document.getElementById(
  "toggle-config",
) as HTMLButtonElement;
const compileBtn = document.getElementById("compile") as HTMLButtonElement;
const runAllBtn = document.getElementById("run-all") as HTMLButtonElement;
const runStatementBtn = document.getElementById(
  "run-statement",
) as HTMLButtonElement;
const runInstructionBtn = document.getElementById(
  "run-instruction",
) as HTMLButtonElement;
const undoStatementBtn = document.getElementById(
  "undo-statement",
) as HTMLButtonElement;
const undoInstructionBtn = document.getElementById(
  "undo-instruction",
) as HTMLButtonElement;
const resetBtn = document.getElementById("reset") as HTMLButtonElement;

const log = document.getElementById("log")!;
const lineEl = document.getElementById("current-line")!;
const memTableEl = document.getElementById("mem-table")!;
const instructionTableEl = document.getElementById("instruction-table")!;
const registerTableEl = document.getElementById("register-table")!;
const variableTableEl = document.getElementById("variables-table")!;

const inputEl = document.getElementById("input") as HTMLInputElement;
const inputBtnEl = document.getElementById("input-btn") as HTMLButtonElement;
const outputEl = document.getElementById("output") as HTMLInputElement;

const resetBeforeCompilation = document.getElementById(
  "compile-resets",
) as HTMLInputElement;

const kwIfInput = document.getElementById("kw-if") as HTMLInputElement;
const kwElseInput = document.getElementById("kw-else") as HTMLInputElement;
const kwWhileInput = document.getElementById("kw-while") as HTMLInputElement;
const kwGotoInput = document.getElementById("kw-goto") as HTMLInputElement;
const kwWriteInput = document.getElementById("kw-write") as HTMLInputElement;
const kwReadInput = document.getElementById("kw-read") as HTMLInputElement;
const kwTrueInput = document.getElementById("kw-true") as HTMLInputElement;
const kwFalseInput = document.getElementById("kw-false") as HTMLInputElement;

// TODO: Make adjustable
const memory = new Memory(2 ** 16);
const input = new Input();
const output = new Output();
const cpu = new Cpu(memory, input, output);

(window as any).mem = memory;
(window as any).cpu = cpu;
(window as any).input = input;
(window as any).output = output;

new LineIndicator(cpu, lineEl);
new MemTable(cpu, memTableEl);
new InstructionTable(cpu, instructionTableEl);
new RegisterTable(cpu, registerTableEl);
new VariableTable(cpu, variableTableEl);

let settingsToggled = false;

setup().then((wrapper) => {
  const { parser, compiler, tokiwen } = wrapper;
  (window as any).parser = parser;
  (window as any).compiler = compiler;
  (window as any).tokiwen = tokiwen;

  function compile() {
    const ast = parser.parse(textarea.value);
    const program = compiler.compile(ast.get());

    const currentProgram = cpu.getProgram();
    if (currentProgram) {
      currentProgram.delete();
    }

    cpu.setProgram(program);
    log.textContent = "";

    ast.delete();
  }

  function compileIfNecessary() {
    if (!cpu.getProgram()) {
      compile();
    }
  }

  function reset() {
    cpu.reset();
    memory.reset(); // This is more to update the memory table than anything
  }

  function lock() {
    compileBtn.disabled = true;
    runAllBtn.disabled = true;
    runStatementBtn.disabled = true;
    runInstructionBtn.disabled = true;
    undoStatementBtn.disabled = true;
    undoInstructionBtn.disabled = true;
    resetBtn.disabled = true;
  }

  function unlock() {
    compileBtn.disabled = false;
    runAllBtn.disabled = false;
    runStatementBtn.disabled = false;
    runInstructionBtn.disabled = false;
    undoStatementBtn.disabled = false;
    undoInstructionBtn.disabled = false;
    resetBtn.disabled = false;
  }

  async function locking(callback: () => Promise<void>) {
    lock();
    try {
      await callback();
    } finally {
      unlock();
    }
  }

  textarea.addEventListener("change", () => {
    if (cpu.getProgram()) {
      log.textContent = "O código foi alterado desde a última compilação.";
    }
  });

  compileBtn.addEventListener("click", () => {
    try {
      if (resetBeforeCompilation.checked) {
        reset();
      }

      compile();
    } catch (err) {
      const casterr = err as Error;
      log.textContent = casterr.message;
      throw err;
    }
  });

  runAllBtn.addEventListener("click", () => {
    try {
      compileIfNecessary();

      locking(async () => {
        await cpu.runProgram();
      });
    } catch (err) {
      log.textContent = "** Runtime Error ** (f12)";
      throw err;
    }
  });

  runStatementBtn.addEventListener("click", () => {
    try {
      compileIfNecessary();

      locking(async () => {
        await cpu.runOneStatement();
      });
    } catch (err) {
      log.textContent = "** Runtime Error ** (f12)";
      throw err;
    }
  });

  runInstructionBtn.addEventListener("click", () => {
    try {
      compileIfNecessary();

      locking(async () => {
        await cpu.runOneInstruction();
      });
    } catch (err) {
      log.textContent = "** Runtime Error ** (f12)";
      throw err;
    }
  });

  undoStatementBtn.addEventListener("click", () => {
    if (!cpu.getProgram()) {
      return;
    }
    cpu.undoOneStatement();
  });

  undoInstructionBtn.addEventListener("click", () => {
    if (!cpu.getProgram()) {
      return;
    }
    cpu.undoOneInstruction();
  });

  resetBtn.addEventListener("click", () => {
    try {
      reset();
    } catch (err) {
      log.textContent = "** Runtime Error ** (f12)";
      throw err;
    }
  });

  input.on("waiting", () => {
    inputBtnEl.disabled = false;
    log.textContent = "Aguardando input.";
  });

  inputBtnEl.addEventListener("click", () => {
    log.textContent = "";
    const asNumber = parseInt(inputEl.value);

    if (!asNumber) {
      const message =
        "Há algo que não é um número no campo de input. Somente números inteiros são suportados no momento.";
      log.textContent = message;
      throw new Error(message);
    }

    const asInt = BigInt(asNumber);
    input.write(asInt);
    inputBtnEl.disabled = true;
  });

  output.on("buffered", async () => {
    outputEl.value = String(await output.read());
  });

  // In case the browser loaded something from cache
  parser.setKeywordIf(kwIfInput.value);
  parser.setKeywordElse(kwElseInput.value);
  parser.setKeywordWhile(kwWhileInput.value);
  parser.setKeywordGoto(kwGotoInput.value);
  parser.setKeywordWrite(kwWriteInput.value);
  parser.setKeywordRead(kwReadInput.value);
  parser.setKeywordTrue(kwTrueInput.value);
  parser.setKeywordFalse(kwFalseInput.value);

  kwIfInput.addEventListener("change", () => {
    parser.setKeywordIf(kwIfInput.value);
  });

  kwElseInput.addEventListener("change", () => {
    parser.setKeywordElse(kwElseInput.value);
  });

  kwWhileInput.addEventListener("change", () => {
    parser.setKeywordWhile(kwWhileInput.value);
  });

  kwGotoInput.addEventListener("change", () => {
    parser.setKeywordGoto(kwGotoInput.value);
  });

  kwWriteInput.addEventListener("change", () => {
    parser.setKeywordWrite(kwWriteInput.value);
  });

  kwReadInput.addEventListener("change", () => {
    parser.setKeywordRead(kwReadInput.value);
  });

  kwTrueInput.addEventListener("change", () => {
    parser.setKeywordTrue(kwTrueInput.value);
  });

  kwFalseInput.addEventListener("change", () => {
    parser.setKeywordFalse(kwFalseInput.value);
  });

  if (settingsToggled) {
    // TODO: Make less magical
    settings.style.height = "83vh";
    settings.style.display = "block";
  }

  settingsBtn.addEventListener("click", () => {
    settingsToggled = !settingsToggled;

    if (settingsToggled) {
      settings.style.display = "block";
    }

    setTimeout(() => {
      settings.style.height = settingsToggled ? "83vh" : "0";
    }, 0);

    setTimeout(() => {
      if (!settingsToggled) {
        settings.style.display = "none";
      }
    }, 300);
  });
});