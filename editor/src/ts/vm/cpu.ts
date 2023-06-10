import { EventEmitter } from "../event-emitter";
import { Program, Instruction } from "../language/program";
import { Variable } from "../language/program-metadata";
import {
  AddExecution,
  AddIExecution,
  AndExecution,
  AndIExecution,
  BranchIfNotZeroExecution,
  BranchIfZeroExecution,
  CallExecution,
  DivideExecution,
  DivideIExecution,
  EqualsExecution,
  ExecutionStack,
  FAddExecution,
  FAddIExecution,
  FDivideExecution,
  FDivideIExecution,
  FMultiplyExecution,
  FMultiplyIExecution,
  FNegateExecution,
  FSubtractExecution,
  FSubtractIExecution,
  GtExecution,
  GteqExecution,
  InstructionExecution,
  InterruptExecution,
  InvertExecution,
  JumpExecution,
  LoadBpExecution,
  LoadExecution,
  LoadIExecution,
  LtExecution,
  LteqExecution,
  MultiplyExecution,
  MultiplyIExecution,
  NegateExecution,
  NoopExecution,
  NotExecution,
  OrExecution,
  OrIExecution,
  PopExecution,
  PushExecution,
  RemainderExecution,
  RemainderIExecution,
  RetExecution,
  SetExecution,
  SubtractExecution,
  SubtractIExecution,
  XorExecution,
  XorIExecution,
} from "./execution-stack";
import { Op } from "./instructions";
import { Input, Output } from "./io";
import { Memory } from "./memory";

type InstructionsMap = {
  [op in Op]: new (cpu: Cpu, operands: bigint[]) => InstructionExecution;
};

type CpuEvents = {
  loadedProgram: [Program];
  updatePc: [bigint];
  updateX: [bigint];
};

export class Cpu extends EventEmitter<CpuEvents> {
  // Registers
  private pc = 0n;
  // private sp = 0n;
  // private bp = 0n;
  private x = 0n;

  private program: Program | undefined;
  private stack: ExecutionStack = new ExecutionStack();
  private instructionIsBoundary: boolean[] = [];

  private readonly instructionMap: InstructionsMap = {
    [Op.NOOP]: NoopExecution,
    [Op.LOAD]: LoadExecution,
    [Op.SET]: SetExecution,
    [Op.LOAD_BP]: LoadBpExecution,
    [Op.LOAD_I]: LoadIExecution,
    [Op.PUSH]: PushExecution,
    [Op.POP]: PopExecution,
    [Op.CALL]: CallExecution,
    [Op.RET]: RetExecution,
    [Op.NEGATE]: NegateExecution,
    [Op.ADD]: AddExecution,
    [Op.SUBTRACT]: SubtractExecution,
    [Op.MULTIPLY]: MultiplyExecution,
    [Op.DIVIDE]: DivideExecution,
    [Op.REMAINDER]: RemainderExecution,
    [Op.ADD_I]: AddIExecution,
    [Op.SUBTRACT_I]: SubtractIExecution,
    [Op.MULTIPLY_I]: MultiplyIExecution,
    [Op.DIVIDE_I]: DivideIExecution,
    [Op.REMAINDER_I]: RemainderIExecution,
    [Op.F_NEGATE]: FNegateExecution,
    [Op.F_ADD]: FAddExecution,
    [Op.F_SUBTRACT]: FSubtractExecution,
    [Op.F_MULTIPLY]: FMultiplyExecution,
    [Op.F_DIVIDE]: FDivideExecution,
    [Op.F_ADD_I]: FAddIExecution,
    [Op.F_SUBTRACT_I]: FSubtractIExecution,
    [Op.F_MULTIPLY_I]: FMultiplyIExecution,
    [Op.F_DIVIDE_I]: FDivideIExecution,
    [Op.OR]: OrExecution,
    [Op.AND]: AndExecution,
    [Op.XOR]: XorExecution,
    [Op.INVERT]: InvertExecution,
    [Op.GT]: GtExecution,
    [Op.LT]: LtExecution,
    [Op.GTEQ]: GteqExecution,
    [Op.LTEQ]: LteqExecution,
    [Op.EQUALS]: EqualsExecution,
    [Op.NOT]: NotExecution,
    [Op.OR_I]: OrIExecution,
    [Op.AND_I]: AndIExecution,
    [Op.XOR_I]: XorIExecution,
    [Op.JUMP]: JumpExecution,
    [Op.BRANCH_IF_ZERO]: BranchIfZeroExecution,
    [Op.BRANCH_IF_NOT_ZERO]: BranchIfNotZeroExecution,
    [Op.INTERRUPT]: InterruptExecution,
  };

  constructor(
    private readonly memory: Memory,
    private readonly input: Input,
    private readonly output: Output,
  ) {
    super();
  }

  public setProgram(program: Program) {
    this.program = program;
    this.memory.load(program.data);

    const { statementBoundaries } = program.metadata;
    this.instructionIsBoundary = new Array(program.instructions.length).fill(
      false,
    );

    for (const boundary of statementBoundaries) {
      this.instructionIsBoundary[Number(boundary)] = true;
    }

    this.emit("loadedProgram", program);
  }

  public getProgram(): Program | undefined {
    return this.program;
  }

  public getMemory() {
    return this.memory;
  }

  public getInput() {
    return this.input;
  }

  public getOutput() {
    return this.output;
  }

  public isDone(): boolean {
    if (!this.program) {
      return false;
    }

    return this.pc >= this.program.instructions.length;
  }

  protected checkForProgram(program: any): asserts program is Program {
    if (!program) {
      throw new Error("Attempted to run program but no program set.");
    }
  }

  public reset() {
    while (!this.stack.isEmpty()) {
      this.stack.pop();
    }
  }

  public async runProgram(): Promise<void> {
    this.checkForProgram(this.program);

    while (!this.isDone()) {
      await this.runOneInstruction();
    }
  }

  public async runOneInstruction(): Promise<void> {
    this.checkForProgram(this.program);
    const { instructions } = this.program;

    if (!this.isDone()) {
      const instruction = instructions[Number(this.pc)];
      await this.runInstruction(instruction);
    }
  }

  public async runInstruction(instruction: Instruction): Promise<void> {
    this.checkForProgram(this.program);

    const op = instruction.op();
    let handler = this.instructionMap[op];

    if (!handler) {
      console.log(
        "Handler not defined for instruction ",
        instruction.mnemonic(),
      );
      handler = NoopExecution;
    }

    const operands = instruction.operands();
    const stackEntry = new handler(this, operands);
    await this.stack.push(stackEntry);
  }

  public async runOneStatement(): Promise<void> {
    this.checkForProgram(this.program);

    do {
      await this.runOneInstruction();
    } while (!this.instructionIsBoundary[Number(this.pc)] && !this.isDone());
  }

  public undoOneInstruction() {
    this.checkForProgram(this.program);

    if (!this.stack.isEmpty()) {
      this.stack.pop();
    }
  }

  public undoOneStatement() {
    this.checkForProgram(this.program);

    do {
      this.undoOneInstruction();
    } while (!this.instructionIsBoundary[Number(this.pc)]);
  }

  protected wrapExecution(
    clazz: new (cpu: Cpu, operands: bigint[]) => InstructionExecution,
  ): (operands: bigint[]) => InstructionExecution {
    return (operands) => new clazz(this, operands);
  }

  public setPc(value: bigint) {
    this.pc = value;
    this.emit("updatePc", value);
  }

  public getPc(): bigint {
    return this.pc;
  }

  public setX(value: bigint) {
    this.x = value;
    this.emit("updateX", value);
  }

  public getX(): bigint {
    return this.x;
  }

  public incPc() {
    this.setPc(this.pc + 1n);
  }

  public decPc() {
    this.setPc(this.pc - 1n);
  }
}
