import { Ast } from "./parser";
import { Program } from "./program";

export class Compiler {
  constructor(private readonly tokiwen: any) {}

  compile(ast: Ast): Program {
    const compiler = new this.tokiwen.Compiler();
    const compilationResult = compiler.compile(ast);

    return new Program(this.tokiwen, compilationResult);
  }
}
