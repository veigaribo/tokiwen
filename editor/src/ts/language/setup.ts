import loadTokiwen from "../../../../build/web/lib/libtokiwen.mjs";
import { Compiler } from "./compiler";
import { Parser } from "./parser";

export interface LanguageWrapper {
  parser: Parser;
  compiler: Compiler;
  tokiwen: any;
}

export async function setup(): Promise<LanguageWrapper> {
  const tokiwen = await loadTokiwen();

  const parser = new Parser(tokiwen);
  const compiler = new Compiler(tokiwen);

  return {
    parser,
    compiler,
    tokiwen,
  };
}
