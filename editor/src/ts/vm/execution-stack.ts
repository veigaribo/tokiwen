import { Cpu } from "./cpu";

// The `undo` method shall return the CPU to the state it was when the object was
// instantiated. Thus only instantiate it immediately before execution.
// Also it is not the responsibility of one object to rewind all of history,
// only to undo what it did.
export abstract class InstructionExecution {
  constructor(
    protected readonly cpu: Cpu,
    protected readonly operands: bigint[],
  ) {}

  abstract do(): Promise<void>;
  abstract undo(): void;

  protected getX(): bigint {
    return this.cpu.getX();
  }

  protected setX(value: bigint): void {
    this.cpu.setX(value);
  }

  protected getPc(): bigint {
    return this.cpu.getPc();
  }

  protected setPc(value: bigint): void {
    this.cpu.setPc(value);
  }

  protected incPc(): void {
    this.cpu.incPc();
  }

  protected decPc(): void {
    this.cpu.decPc();
  }

  protected readMem(addr: bigint): bigint {
    const memory = this.cpu.getMemory();
    return memory.read(Number(addr));
  }

  protected writeMem(addr: bigint, value: bigint): void {
    const memory = this.cpu.getMemory();
    return memory.write(Number(addr), value);
  }

  protected output(value: bigint): void {
    this.cpu.getOutput().write(value);
  }

  protected input(): Promise<bigint> {
    return this.cpu.getInput().read();
  }
}

// If the instruction increments PC and nothing more
export abstract class NoopInstructionExecution extends InstructionExecution {
  constructor(cpu: Cpu, operands: bigint[]) {
    super(cpu, operands);
  }

  undo() {
    this.decPc();
  }
}

// If the instruction changes X and nothing more
export abstract class XInstructionExecution extends InstructionExecution {
  private readonly previousX: bigint;

  constructor(cpu: Cpu, operands: bigint[]) {
    super(cpu, operands);
    this.previousX = cpu.getX();
  }

  undo() {
    this.decPc();
    this.setX(this.previousX);
  }
}

// If the instruction changes PC and nothing more
export abstract class PcInstructionExecution extends InstructionExecution {
  private readonly previousPc: bigint;

  constructor(cpu: Cpu, operands: bigint[]) {
    super(cpu, operands);
    this.previousPc = cpu.getPc();
  }

  undo() {
    this.setPc(this.previousPc);
  }
}

export class ExecutionStack {
  private readonly stack: InstructionExecution[];

  constructor() {
    this.stack = [];
  }

  async push(execution: InstructionExecution): Promise<void> {
    await execution.do();
    this.stack.push(execution);
  }

  pop(): InstructionExecution {
    if (this.isEmpty()) {
      throw new Error("Tried to undo instruction with empty stack");
    }

    const execution = this.stack.pop()!;
    execution.undo();
    return execution;
  }

  isEmpty() {
    return this.stack.length === 0;
  }
}

export class NoopExecution extends NoopInstructionExecution {
  async do() {
    this.incPc();
  }
}

export class LoadExecution extends XInstructionExecution {
  async do() {
    const [addr] = this.operands;

    this.setX(this.readMem(addr));
    this.incPc();
  }
}

export class SetExecution extends InstructionExecution {
  private readonly previousMem: bigint;

  constructor(cpu: Cpu, operands: bigint[]) {
    super(cpu, operands);
    const [addr] = this.operands;
    this.previousMem = this.readMem(addr);
  }

  async do() {
    const [addr] = this.operands;
    this.writeMem(addr, this.getX());

    this.incPc();
  }

  undo() {
    this.decPc();

    const [addr] = this.operands;
    this.writeMem(addr, this.previousMem);
  }
}

export class LoadBpExecution extends NoopInstructionExecution {
  async do() {
    // Not supported at the moment.
    // this.getX() = this.bp;
    this.incPc();
  }
}

export class LoadIExecution extends XInstructionExecution {
  async do() {
    const [value] = this.operands;

    this.setX(value);
    this.incPc();
  }
}

export class PushExecution extends NoopInstructionExecution {
  async do() {
    // Not supported at the moment.
    this.incPc();
  }
}

export class PopExecution extends NoopInstructionExecution {
  async do() {
    // Not supported at the moment.
    this.incPc();
  }
}

export class CallExecution extends NoopInstructionExecution {
  async do() {
    // Not supported at the moment. :c
    this.incPc();
  }
}

export class RetExecution extends NoopInstructionExecution {
  async do() {
    // Not supported at the moment. :c
    this.incPc();
  }
}

export class NegateExecution extends XInstructionExecution {
  async do() {
    this.setX(-this.getX());
    this.incPc();
  }
}

export class AddExecution extends XInstructionExecution {
  async do() {
    const [addr] = this.operands;
    const y = this.readMem(addr);

    this.setX(BigInt.asIntN(64, y + this.getX()));
    this.incPc();
  }
}

export class SubtractExecution extends XInstructionExecution {
  async do() {
    const [addr] = this.operands;
    const y = this.readMem(addr);

    this.setX(BigInt.asIntN(64, y - this.getX()));
    this.incPc();
  }
}

export class MultiplyExecution extends XInstructionExecution {
  async do() {
    const [addr] = this.operands;
    const y = this.readMem(addr);

    this.setX(BigInt.asIntN(64, y * this.getX()));
    this.incPc();
  }
}

function formatDivisionByZeroError(cpu: Cpu): string {
  const instruction = cpu.getPc();
  const line = cpu.getProgram()!.metadata.sourceLines[Number(instruction)];
  return `Division by zero! at instruction ${instruction}, line ${line}.`;
}

export class DivideExecution extends XInstructionExecution {
  async do() {
    if (this.getX() === 0n) {
      throw new Error(formatDivisionByZeroError(this.cpu));
    }

    const [addr] = this.operands;
    const y = this.readMem(addr);

    this.setX(BigInt.asIntN(64, y / this.getX()));
    this.incPc();
  }
}

export class RemainderExecution extends XInstructionExecution {
  async do() {
    if (this.getX() === 0n) {
      throw new Error(formatDivisionByZeroError(this.cpu));
    }

    const [addr] = this.operands;
    const y = this.readMem(addr);

    this.setX(BigInt.asIntN(64, y % this.getX()));
    this.incPc();
  }
}

export class AddIExecution extends XInstructionExecution {
  async do() {
    const [value] = this.operands;

    this.setX(BigInt.asIntN(64, this.getX() + value));
    this.incPc();
  }
}

export class SubtractIExecution extends XInstructionExecution {
  async do() {
    const [value] = this.operands;

    this.setX(BigInt.asIntN(64, this.getX() - value));
    this.incPc();
  }
}

export class MultiplyIExecution extends XInstructionExecution {
  async do() {
    const [value] = this.operands;

    this.setX(BigInt.asIntN(64, this.getX() * value));
    this.incPc();
  }
}

export class DivideIExecution extends XInstructionExecution {
  async do() {
    const [value] = this.operands;

    if (value === 0n) {
      throw new Error(formatDivisionByZeroError(this.cpu));
    }

    this.setX(BigInt.asIntN(64, this.getX() / value));
    this.incPc();
  }
}

export class RemainderIExecution extends XInstructionExecution {
  async do() {
    const [value] = this.operands;

    if (value === 0n) {
      throw new Error(formatDivisionByZeroError(this.cpu));
    }

    this.setX(BigInt.asIntN(64, this.getX() % value));
    this.incPc();
  }
}

export class FNegateExecution extends XInstructionExecution {
  async do() {
    // Not supported at the moment.
    this.setX(0xbadn);
    this.incPc();
  }
}

export class FAddExecution extends XInstructionExecution {
  async do() {
    // Not supported at the moment.
    this.setX(0xbadn);
    this.incPc();
  }
}

export class FSubtractExecution extends XInstructionExecution {
  async do() {
    // Not supported at the moment.
    this.setX(0xbadn);
    this.incPc();
  }
}

export class FMultiplyExecution extends XInstructionExecution {
  async do() {
    // Not supported at the moment.
    this.setX(0xbadn);
    this.incPc();
  }
}

export class FDivideExecution extends XInstructionExecution {
  async do() {
    // Not supported at the moment.
    this.setX(0xbadn);
    this.incPc();
  }
}

export class FAddIExecution extends XInstructionExecution {
  async do() {
    // Not supported at the moment.
    this.setX(0xbadn);
    this.incPc();
  }
}

export class FSubtractIExecution extends XInstructionExecution {
  async do() {
    // Not supported at the moment.
    this.setX(0xbadn);
    this.incPc();
  }
}

export class FMultiplyIExecution extends XInstructionExecution {
  async do() {
    // Not supported at the moment.
    this.setX(0xbadn);
    this.incPc();
  }
}

export class FDivideIExecution extends XInstructionExecution {
  async do() {
    // Not supported at the moment.
    this.setX(0xbadn);
    this.incPc();
  }
}

export class OrExecution extends XInstructionExecution {
  async do() {
    const [addr] = this.operands;
    const y = this.readMem(addr);

    this.setX(y | this.getX());
    this.incPc();
  }
}

export class AndExecution extends XInstructionExecution {
  async do() {
    const [addr] = this.operands;
    const y = this.readMem(addr);

    this.setX(y & this.getX());
    this.incPc();
  }
}

export class XorExecution extends XInstructionExecution {
  async do() {
    const [addr] = this.operands;
    const y = this.readMem(addr);

    this.setX(y ^ this.getX());
    this.incPc();
  }
}

export class InvertExecution extends XInstructionExecution {
  async do() {
    this.setX(~this.getX());
    this.incPc();
  }
}

export class GtExecution extends XInstructionExecution {
  async do() {
    const [addr] = this.operands;
    const y = this.readMem(addr);

    this.setX(y > this.getX() ? 1n : 0n);
    this.incPc();
  }
}

export class LtExecution extends XInstructionExecution {
  async do() {
    const [addr] = this.operands;
    const y = this.readMem(addr);

    this.setX(y < this.getX() ? 1n : 0n);
    this.incPc();
  }
}

export class GteqExecution extends XInstructionExecution {
  async do() {
    const [addr] = this.operands;
    const y = this.readMem(addr);

    this.setX(y >= this.getX() ? 1n : 0n);
    this.incPc();
  }
}

export class LteqExecution extends XInstructionExecution {
  async do() {
    const [addr] = this.operands;
    const y = this.readMem(addr);

    this.setX(y <= this.getX() ? 1n : 0n);
    this.incPc();
  }
}

export class EqualsExecution extends XInstructionExecution {
  async do() {
    const [addr] = this.operands;
    const y = this.readMem(addr);

    this.setX(y === this.getX() ? 1n : 0n);
    this.incPc();
  }
}

export class NotExecution extends XInstructionExecution {
  async do() {
    this.setX(this.getX() === 0n ? 1n : 0n);
    this.incPc();
  }
}

export class OrIExecution extends XInstructionExecution {
  async do() {
    const [value] = this.operands;

    this.setX(this.getX() | value);
    this.incPc();
  }
}

export class AndIExecution extends XInstructionExecution {
  async do() {
    const [value] = this.operands;

    this.setX(this.getX() & value);
    this.incPc();
  }
}

export class XorIExecution extends XInstructionExecution {
  async do() {
    const [value] = this.operands;

    this.setX(this.getX() ^ value);
    this.incPc();
  }
}

export class JumpExecution extends PcInstructionExecution {
  async do() {
    const [addr] = this.operands;
    this.setPc(addr);
  }
}

export class BranchIfZeroExecution extends PcInstructionExecution {
  async do() {
    const [addr] = this.operands;

    if (this.getX() === 0n) {
      this.setPc(addr);
    } else {
      this.incPc();
    }
  }
}

export class BranchIfNotZeroExecution extends PcInstructionExecution {
  async do() {
    const [addr] = this.operands;

    if (this.getX() !== 0n) {
      this.setPc(addr);
    } else {
      this.incPc();
    }
  }
}

export class InterruptExecution extends XInstructionExecution {
  async runInput() {
    const addr = this.getX();
    const value = await this.input();
    this.writeMem(addr, value);
  }

  async runOutput() {
    const value = this.getX();
    this.output(value);
  }

  async do() {
    const [op] = this.operands;

    // Syscalls
    switch (op) {
      case 0n:
        await this.runInput();
        break;
      case 1n:
        await this.runOutput();
        break;
    }

    this.incPc();
  }
}
