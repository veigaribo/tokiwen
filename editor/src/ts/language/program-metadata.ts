export class Position {
  public readonly line: bigint;
  public readonly column: bigint;

  constructor(tokiwenPosition: any) {
    this.line = tokiwenPosition.line;
    this.column = tokiwenPosition.column;
  }

  format(): string {
    return `${this.line}.${this.column}`;
  }
}

export class Location {
  public readonly begin: Position;
  public readonly end: Position;

  constructor(tokiwenLocation: any) {
    this.begin = new Position(tokiwenLocation.begin);
    this.end = new Position(tokiwenLocation.end);
  }

  format(): string {
    return `:${this.begin.format()}`;
  }
}

export class Variable {
  public readonly name: string;
  public readonly size: bigint;
  public readonly address: bigint;
  public readonly declaredAt: Location;

  constructor(tokiwenVariable: any) {
    this.name = tokiwenVariable.name;
    this.size = tokiwenVariable.size;
    this.address = tokiwenVariable.address;
    this.declaredAt = new Location(tokiwenVariable.declaredAt);
  }
}

export class ProgramMetadata {
  public readonly statementBoundaries: bigint[] = [];
  public readonly variables: Map<number, Variable> = new Map();
  public readonly sourceLines: bigint[] = [];

  constructor(tokiwenProgramMetadata: any) {
    for (
      var i = 0;
      i < tokiwenProgramMetadata.statementBoundaries.size();
      ++i
    ) {
      this.statementBoundaries[i] =
        tokiwenProgramMetadata.statementBoundaries.get(i);
    }

    const keys = tokiwenProgramMetadata.variables.keys();
    for (var i = 0; i < keys.size(); ++i) {
      const key = keys.get(i);
      this.variables.set(
        Number(key),
        new Variable(tokiwenProgramMetadata.variables.get(key)),
      );
    }

    for (var i = 0; i < tokiwenProgramMetadata.sourceLineMap.size(); ++i) {
      this.sourceLines[i] = tokiwenProgramMetadata.sourceLineMap.get(i);
    }
  }
}
