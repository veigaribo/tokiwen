import { Cpu } from "../vm/cpu";

export class RegisterTable {
  private pcEl: HTMLElement;
  private xEl: HTMLElement;

  constructor(cpu: Cpu, private readonly el: HTMLElement) {
    const header = document.createElement("TR");
    const pcTh = document.createElement("TH");
    pcTh.textContent = "PC";
    const xTh = document.createElement("TH");
    xTh.textContent = "X";

    header.append(pcTh, xTh);
    this.el.append(header);

    const values = document.createElement("TR");
    this.pcEl = document.createElement("TD");
    this.pcEl.textContent = "0";
    this.xEl = document.createElement("TD");
    this.xEl.textContent = "0";
    values.append(this.pcEl, this.xEl);
    this.el.append(values);

    cpu.on("updatePc", this.updatePc.bind(this));
    cpu.on("updateX", this.updateX.bind(this));
  }

  private updatePc(value: bigint) {
    this.pcEl.textContent = String(value);
  }

  private updateX(value: bigint) {
    this.xEl.textContent = String(value);
  }
}
