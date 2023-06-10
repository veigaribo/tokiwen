import { Program } from "../language/program";
import { Cpu } from "../vm/cpu";
import { instructionDescription } from "../vm/instructions";

export class InstructionTable {
  private rows: Map<bigint, HTMLElement> = new Map();
  private current: bigint = 0n;
  private tooltipIdCounter = 0;

  constructor(cpu: Cpu, private readonly el: HTMLElement) {
    this.init();

    cpu.on("loadedProgram", this.loadProgram.bind(this));
    cpu.on("updatePc", this.updateCurrent.bind(this));
  }

  private init() {
    const header = document.createElement("TR");
    const currentTh = document.createElement("TH");
    currentTh.textContent = "Próxima";
    const addrTh = document.createElement("TH");
    addrTh.textContent = "Índice";
    const valueTh = document.createElement("TH");
    valueTh.textContent = "Mnemônico";

    header.append(currentTh, addrTh, valueTh);
    this.el.append(header);
  }

  private makeTooltipText(mnemonic: string, description: string): string {
    return `${mnemonic}: ${description}`;
  }

  private makeTooltip(tooltipText: string): string {
    const tooltip = document.createElement("DIV");
    tooltip.textContent = tooltipText;
    tooltip.style.display = "none";
    tooltip.style.position = "fixed";
    tooltip.style.top = "calc(3px + 1em + 3px + 1em + 3px)";

    const id = `instruction-tooltip-${this.tooltipIdCounter++}`;
    tooltip.id = id;

    document.body.appendChild(tooltip);
    return id;
  }

  private loadProgram(program: Program) {
    const instructions = program.instructions;

    // Clear
    for (const [_, row] of this.rows) {
      const tooltipId = row.getAttribute("data-tooltip");

      if (tooltipId) {
        const tooltip = document.getElementById(tooltipId);

        if (tooltip) {
          document.body.removeChild(tooltip);
        }
      }

      this.el.removeChild(row);
    }

    this.rows = new Map();

    let index = 0n;
    for (const instruction of instructions) {
      const newRow = document.createElement("TR");
      const description = instructionDescription[instruction.op()];
      const tooltipText = this.makeTooltipText(
        instruction.mnemonic(),
        description,
      );
      newRow.title = tooltipText;

      if (window.innerWidth < 1080) {
        newRow.tabIndex = 0; // To allow focus

        const tooltip = this.makeTooltip(tooltipText);
        newRow.setAttribute("data-tooltip", tooltip);

        newRow.addEventListener("focus", function () {
          const tooltipId = this.getAttribute("data-tooltip")!;
          const tooltip = document.getElementById(tooltipId)!;
          tooltip.style.display = "block";
        });

        newRow.addEventListener("blur", function () {
          const tooltipId = this.getAttribute("data-tooltip")!;
          const tooltip = document.getElementById(tooltipId)!;
          tooltip.style.display = "none";
        });
      }

      const currentEl = document.createElement("TD");
      currentEl.textContent = index === this.current ? "*" : "";
      const indexEl = document.createElement("TD");
      indexEl.textContent = String(index);

      const mnemonicEl = document.createElement("TD");
      let stringed = instruction.mnemonic();

      for (var i = 0; i < instruction.operandCount(); ++i) {
        stringed += "\t" + instruction.operand(i);
      }

      mnemonicEl.textContent = stringed;

      newRow.append(currentEl, indexEl, mnemonicEl);
      this.rows.set(index, newRow);

      this.el.append(newRow);
      ++index;
    }

    // One extra row to indicate that the program has terminated
    const newRow = document.createElement("TR");

    const currentEl = document.createElement("TD");
    currentEl.textContent = index === this.current ? "*" : "";
    const indexEl = document.createElement("TD");
    indexEl.textContent = String(index);
    const mnemonicEl = document.createElement("TD");
    mnemonicEl.textContent = "";

    newRow.append(currentEl, indexEl, mnemonicEl);
    this.rows.set(index, newRow);

    this.el.append(newRow);
  }

  private updateCurrent(index: bigint) {
    const currentCurrent = this.rows.get(this.current);

    if (currentCurrent) {
      currentCurrent.firstChild!.textContent = "";
    }

    const newCurrent = this.rows.get(index);

    if (newCurrent) {
      newCurrent.firstChild!.textContent = "*";
    }

    this.current = index;
  }
}
