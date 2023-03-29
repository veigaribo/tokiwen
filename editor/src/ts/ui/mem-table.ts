import { Cpu } from "../vm/cpu";
import { Variable } from "../language/program-metadata";
import { Program } from "../language/program";

export class MemTable {
  private rows: Map<number, HTMLElement> = new Map();
  private maxAddr: number;
  private variableBytes: Map<number, Variable>;

  constructor(cpu: Cpu, private readonly el: HTMLElement) {
    const memory = cpu.getMemory();

    this.maxAddr = memory.getSize();
    this.variableBytes = new Map();

    this.init();
    memory.on("reset", this.clear.bind(this));
    memory.on("writeByte", this.write.bind(this));
    cpu.on("loadedProgram", this.addVariables.bind(this));
  }

  private init() {
    const header = document.createElement("TR");
    const varTh = document.createElement("TH");
    varTh.textContent = "Variável";
    const addrTh = document.createElement("TH");
    addrTh.textContent = "Endereço";
    const valueTh = document.createElement("TH");
    valueTh.textContent = "Conteúdo";

    header.append(varTh, addrTh, valueTh);
    this.el.append(header);
  }

  private clear() {
    for (const [_, row] of this.rows) {
      this.el.removeChild(row);
    }
    this.rows.clear();
  }

  private write(addr: number, value: bigint): void {
    this.updateRow(addr, value);
  }

  private updateRow(addr: number, value: bigint): void {
    let existingRow = this.rows.get(addr);

    const newRow = document.createElement("TR");

    const varEl = document.createElement("TD");

    if (this.variableBytes.has(addr)) {
      const variable = this.variableBytes.get(addr)!;
      varEl.textContent = String(variable.name);
    }

    const addrEl = document.createElement("TD");
    addrEl.textContent = String(addr);
    const valueEl = document.createElement("TD");
    valueEl.textContent = String(value);

    newRow.append(varEl, addrEl, valueEl);

    if (existingRow) {
      existingRow.replaceWith(newRow);
    } else {
      this.positionRow(addr, newRow);
    }

    this.rows.set(addr, newRow);
  }

  private positionRow(addr: number, row: HTMLElement): void {
    if (this.rows.size === 0) {
      this.el.append(row);
      return;
    }

    let below = addr;
    while (below >= 0 && !this.rows.has(below)) {
      --below;
    }

    let above = addr;
    while (above <= this.maxAddr && !this.rows.has(above)) {
      ++above;
    }

    let downdiff = below < 0 ? Infinity : addr - below;
    let updiff = above > this.maxAddr ? Infinity : above - addr;

    if (downdiff < updiff) {
      this.rows.get(below)!.after(row);
    } else {
      this.rows.get(above)!.before(row);
    }
  }

  private addVariables(program: Program) {
    this.variableBytes.clear();

    const variables = program.metadata.variables.values();
    for (const variable of variables) {
      const addr = Number(variable.address);

      for (var i = addr; i < addr + Number(variable.size); ++i) {
        this.variableBytes.set(i, variable);
      }
    }
  }
}
