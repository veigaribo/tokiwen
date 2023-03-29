// Opaque
export class Ast {
  constructor(private readonly tokiwenAst: any) {}

  get(): any {
    return this.tokiwenAst;
  }

  delete() {
    this.tokiwenAst.delete();
  }
}

export class Parser {
  private ifKeyword = "if";
  private elseKeyword = "else";
  private whileKeyword = "while";
  private gotoKeyword = "goto";
  private writeKeyword = "write";
  private readKeyword = "read";
  private trueKeyword = "true";
  private falseKeyword = "false";

  constructor(private readonly tokiwen: any) {}

  parse(source: string): Ast {
    const tparser = new this.tokiwen.Parser(source);
    tparser.setKeyword(this.tokiwen.Keyword.If, this.ifKeyword);
    tparser.setKeyword(this.tokiwen.Keyword.Else, this.elseKeyword);
    tparser.setKeyword(this.tokiwen.Keyword.While, this.whileKeyword);
    tparser.setKeyword(this.tokiwen.Keyword.Goto, this.gotoKeyword);
    tparser.setKeyword(this.tokiwen.Keyword.Write, this.writeKeyword);
    tparser.setKeyword(this.tokiwen.Keyword.Read, this.readKeyword);
    tparser.setKeyword(this.tokiwen.Keyword.True, this.trueKeyword);
    tparser.setKeyword(this.tokiwen.Keyword.False, this.falseKeyword);

    const parseResult = tparser.parse();

    if (!parseResult.success) {
      throw new Error("Parsing failed.\n" + parseResult.message);
    }

    return new Ast(parseResult.ast);
  }

  setKeywordIf(to: string) {
    this.ifKeyword = to;
  }

  setKeywordElse(to: string) {
    this.elseKeyword = to;
  }

  setKeywordWhile(to: string) {
    this.whileKeyword = to;
  }

  setKeywordGoto(to: string) {
    this.gotoKeyword = to;
  }

  setKeywordWrite(to: string) {
    this.writeKeyword = to;
  }

  setKeywordRead(to: string) {
    this.readKeyword = to;
  }

  setKeywordTrue(to: string) {
    this.trueKeyword = to;
  }

  setKeywordFalse(to: string) {
    this.falseKeyword = to;
  }
}
