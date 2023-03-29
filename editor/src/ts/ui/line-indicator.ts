import { Program } from "../language/program";
import { Cpu } from "../vm/cpu";

export class LineIndicator {
  private readonly header: HTMLElement;
  private sourceLines: bigint[] = [];

  constructor(cpu: Cpu, private readonly el: HTMLElement) {
    this.header = document.createElement("SPAN");
    this.header.textContent = "Nenhum programa.";

    this.el.append(this.header);

    cpu.on("loadedProgram", this.loadProgram.bind(this));
    cpu.on("updatePc", this.updateLine.bind(this));
  }

  private formatLine(line: bigint): string {
    return `Linha da próxima instrução: ${line}`;
  }

  private loadProgram(program: Program) {
    this.sourceLines = program.metadata.sourceLines;
    const line = this.sourceLines[0];
    this.header.textContent = this.formatLine(line);
  }

  private updateLine(index: bigint) {
    if (index >= this.sourceLines.length) {
      this.header.textContent = "Finalizado.";
      return;
    }

    const line = this.sourceLines[Number(index)];
    this.header.textContent = this.formatLine(line);
  }
}
