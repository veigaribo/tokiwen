import { Cpu } from "../vm/cpu";
import { Variable } from "../language/program-metadata";
import { Memory } from "../vm/memory";
import { Program } from "../language/program";

export class VariableTable {
  private varEls: Map<string, HTMLElement>;
  private vars: Map<string, Variable>;
  private mem: Memory;

  constructor(cpu: Cpu, private readonly el: HTMLElement) {
    this.varEls = new Map();
    this.vars = new Map();
    this.mem = cpu.getMemory();
    this.init();

    cpu.on("loadedProgram", this.updateVars.bind(this));
    this.mem.on("write", this.updateVarValue.bind(this));
    this.mem.on("reset", this.updateVarValue.bind(this));
  }

  private init() {
    const header = document.createElement("TR");
    const nameTh = document.createElement("TH");
    nameTh.textContent = "Nome";
    const addrTh = document.createElement("TH");
    addrTh.textContent = "Endereço";
    const sizeTh = document.createElement("TH");
    sizeTh.textContent = "Tamanho";
    const valueTh = document.createElement("TH");
    valueTh.textContent = "Conteúdo";

    header.append(nameTh, addrTh, sizeTh, valueTh);
    this.el.append(header);
  }

  private getId(variable: Variable): string {
    return variable.name + variable.declaredAt.format();
  }

  private clear() {
    for (const [_, row] of this.varEls) {
      this.el.removeChild(row);
    }

    this.varEls.clear();
    this.vars.clear();
  }

  private updateVars(program: Program) {
    this.clear();

    const variables = program.metadata.variables.values();
    for (const variable of variables) {
      const id = this.getId(variable);

      if (this.varEls.has(id)) {
        const current = this.varEls.get(id)!;
        this.el.removeChild(current);
      }

      const newRow = document.createElement("TR");
      const nameEl = document.createElement("TD");
      nameEl.textContent = id;
      const addressEl = document.createElement("TD");
      addressEl.textContent = String(variable.address);
      const sizeEl = document.createElement("TD");
      sizeEl.textContent = String(variable.size);
      const valueEl = document.createElement("TD");

      const value64 = this.mem.read(Number(variable.address));
      const valueTrimmed = BigInt.asIntN(Number(variable.size) * 8, value64);
      valueEl.textContent = String(valueTrimmed);

      newRow.append(nameEl, addressEl, sizeEl, valueEl);
      this.varEls.set(id, newRow);
      this.vars.set(id, variable);

      this.el.append(newRow);
    }
  }

  private updateVarValue() {
    for (const [_, variable] of this.vars) {
      const id = this.getId(variable);
      const el = this.varEls.get(id)!;

      const valueEl = el.children[3];
      const value64 = this.mem.read(Number(variable.address));
      const valueTrimmed = BigInt.asIntN(Number(variable.size) * 8, value64);
      valueEl.textContent = String(valueTrimmed);
    }
  }
}
