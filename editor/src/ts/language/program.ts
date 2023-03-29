import { Op } from "../vm/instructions";
import { ProgramMetadata } from "./program-metadata";

export class Operation {
  constructor(private readonly tokiwen: any, private readonly tokiwenOp: any) {}

  opcode(): number {
    return this.tokiwen.opcodeOfInstruction(this.tokiwenOp);
  }
  op(): Op {
    return this.opcode() as Op;
  }
  mnemonic(): string {
    return this.tokiwen.nameOfInstruction(this.tokiwenOp);
  }
  operandCount(): number {
    return this.tokiwen.operandCountOfInstruction(this.tokiwenOp);
  }
}

export class Instruction {
  private readonly operation: Operation;

  constructor(
    private readonly tokiwen: any,
    private readonly tokiwenInstruction: any,
  ) {
    this.operation = new Operation(
      this.tokiwen,
      this.tokiwenInstruction.getOperation(),
    );
  }

  getOperation(): Operation {
    return this.operation;
  }

  opcode(): number {
    return this.getOperation().opcode();
  }

  op(): Op {
    return this.getOperation().op();
  }

  mnemonic(): string {
    return this.getOperation().mnemonic();
  }

  operandCount(): number {
    return this.getOperation().operandCount();
  }

  operand(index: number): bigint {
    return this.tokiwenInstruction.getOperand(index);
  }

  operands(): bigint[] {
    const accum = [];

    for (var i = 0; i < this.operandCount(); ++i) {
      accum.push(this.operand(i));
    }

    return accum;
  }
}

export class Program {
  public readonly data: Uint8Array;
  public readonly instructions: Instruction[];
  public readonly metadata: ProgramMetadata;

  constructor(tokiwen: any, private readonly tokiwenProgram: any) {
    const dataSize = tokiwenProgram.data.size();
    this.data = new Uint8Array(dataSize);

    // This is as std::vector so no fancy stuff
    for (var i = 0; i < tokiwenProgram.data.size(); ++i) {
      this.data[i] = tokiwenProgram.data.get(i);
    }

    this.instructions = [];

    for (var i = 0; i < tokiwenProgram.code.size(); ++i) {
      const instruction = tokiwenProgram.code.get(i);

      const wrapped = new Instruction(tokiwen, instruction);
      this.instructions.push(wrapped);
    }

    this.metadata = new ProgramMetadata(tokiwenProgram.metadata);
  }

  delete() {
    this.tokiwenProgram.delete();
  }
}
