export enum Op {
  NOOP = 0,
  LOAD = 1,
  SET = 2,
  LOAD_BP = 3,
  LOAD_I = 4,

  PUSH = 10,
  POP = 11,
  CALL = 12,
  RET = 13,

  NEGATE = 100,
  ADD = 101,
  SUBTRACT = 102,
  MULTIPLY = 103,
  DIVIDE = 104,
  REMAINDER = 105,

  ADD_I = 106,
  SUBTRACT_I = 107,
  MULTIPLY_I = 108,
  DIVIDE_I = 109,
  REMAINDER_I = 110,

  F_NEGATE = 111,
  F_ADD = 112,
  F_SUBTRACT = 113,
  F_MULTIPLY = 114,
  F_DIVIDE = 115,

  F_ADD_I = 116,
  F_SUBTRACT_I = 117,
  F_MULTIPLY_I = 118,
  F_DIVIDE_I = 119,

  OR = 160,
  AND = 161,
  XOR = 162,
  INVERT = 163,

  GT = 164,
  LT = 165,
  GTEQ = 166,
  LTEQ = 167,
  EQUALS = 168,
  NOT = 169,

  OR_I = 170,
  AND_I = 171,
  XOR_I = 172,

  JUMP = 200,
  BRANCH_IF_ZERO = 201,
  BRANCH_IF_NOT_ZERO = 202,

  INTERRUPT = 255,
}

type OpDescription = {
  [op in Op]: string;
};

export const instructionDescription: OpDescription = {
  [Op.NOOP]: "Nada acontece.",
  [Op.LOAD]: "O endereço do operando é carregado no registrador X.",
  [Op.SET]: "O valor do registrador X é salvo no endereço do operando.",
  [Op.LOAD_BP]: "Não implementado.",
  [Op.LOAD_I]: "O valor do operando é carregado no registrador X.",
  [Op.PUSH]: "Não implementado.",
  [Op.POP]: "Não implementado.",
  [Op.CALL]: "Não implementado.",
  [Op.RET]: "Não implementado.",
  [Op.NEGATE]:
    "Nega (torna negativo se positivo ou positivo se negativo) o valor no registrador X.",
  [Op.ADD]:
    "Soma o valor no endereço do operando com o valor no registrador X. O resultado é armazenado no registrador X.",
  [Op.SUBTRACT]:
    "Subtrai o valor no endereço do operando do valor no registrador X. O resultado é armazenado no registrador X.",
  [Op.MULTIPLY]:
    "Multiplica o valor no endereço do operando com o valor no registrador X. O resultado é armazenado no registrador X.",
  [Op.DIVIDE]:
    "Divide o valor no endereço do operando pelo o valor no registrador X. O resultado é armazenado no registrador X.",
  [Op.REMAINDER]:
    "Calcula o resto da divisão inteira do valor no endereço do operando com o valor no registrador X. O resultado é armazenado no registrador X.",
  [Op.ADD_I]:
    "Soma o valor no operando com o valor no registrador X. O resultado é armazenado no registrador X.",
  [Op.SUBTRACT_I]:
    "Subtrai o valor no operando do valor no registrador X. O resultado é armazenado no registrador X.",
  [Op.MULTIPLY_I]:
    "Multiplica o valor no operando com o valor no registrador X. O resultado é armazenado no registrador X.",
  [Op.DIVIDE_I]:
    "Divide o valor no operando pelo valor no registrador X. O resultado é armazenado no registrador X.",
  [Op.REMAINDER_I]:
    "Calcula o resto da divisão inteira do valor no operando com o valor no registrador X. O resultado é armazenado no registrador X.",
  [Op.F_NEGATE]: "Não implementado.",
  [Op.F_ADD]: "Não implementado.",
  [Op.F_SUBTRACT]: "Não implementado.",
  [Op.F_MULTIPLY]: "Não implementado.",
  [Op.F_DIVIDE]: "Não implementado.",
  [Op.F_ADD_I]: "Não implementado.",
  [Op.F_SUBTRACT_I]: "Não implementado.",
  [Op.F_MULTIPLY_I]: "Não implementado.",
  [Op.F_DIVIDE_I]: "Não implementado.",
  [Op.OR]:
    "Realiza o operador lógico `ou` no valor no endereço do operando e no valor no registrador X bit a bit. Caso um dos dois bits seja 1, o respectivo bit em X se torna 1. Caso contrário, se torna 0.",
  [Op.AND]:
    "Realiza o operador lógico `e` no valor no endereço do operando e no valor no registrador X bit a bit. Caso ambos bits sejam 1, o respectivo bit em X se torna 1. Caso contrário, se torna 0.",
  [Op.XOR]:
    "Realiza o operador lógico `ou exclusivo` no valor no endereço do operando e no valor no registrador X bit a bit. Caso ambos bits sejam diferentes, o respectivo bit em X se torna 1. Caso contrário, se torna 0.",
  [Op.INVERT]:
    "Inverte os valores dos bits do valor no registrador X. 0 se torna 1 e 1 se torna 0.",
  [Op.GT]:
    "Compara o valor no endereço do operando com o valor no registrador X. Se o primeiro for maior que o segundo, o valor no registrador X se torna 1. Caso contrário. se torna 0.",
  [Op.LT]:
    "Compara o valor no endereço do operando com o valor no registrador X. Se o primeiro for menor que o segundo, o valor no registrador X se torna 1. Caso contrário. se torna 0.",
  [Op.GTEQ]:
    "Compara o valor no endereço do operando com o valor no registrador X. Se o primeiro for maior que ou igual ao segundo, o valor no registrador X se torna 1. Caso contrário. se torna 0.",
  [Op.LTEQ]:
    "Compara o valor no endereço do operando com o valor no registrador X. Se o primeiro for menor que ou igual ao segundo, o valor no registrador X se torna 1. Caso contrário. se torna 0.",
  [Op.EQUALS]:
    "Compara o valor no endereço do operando com o valor no registrador X. Se o primeiro for igual ao segundo, o valor no registrador X se torna 1. Caso contrário. se torna 0.",
  [Op.NOT]:
    "Inverte logicamente o valor no registrador X. 0 se torna 1, outros valores se tornam 0.",
  [Op.OR_I]:
    "Realiza o operador lógico `ou` no valor do operando e no valor no registrador X bit a bit. Caso um dos dois bits seja 1, o respectivo bit em X se torna 1. Caso contrário, se torna 0.",
  [Op.AND_I]:
    "Realiza o operador lógico `e` no valor do operando e no valor no registrador X bit a bit. Caso ambos bits sejam 1, o respectivo bit em X se torna 1. Caso contrário, se torna 0.",
  [Op.XOR_I]:
    "Realiza o operador lógico `ou exclusivo` no valor do operando e no valor no registrador X bit a bit. Caso ambos bits sejam diferentes, o respectivo bit em X se torna 1. Caso contrário, se torna 0.",
  [Op.JUMP]:
    "Carrega o valor do operando no registrador PC, efetivamente fazendo com que a instrução nesse índice se torne a próxima a ser executada.",
  [Op.BRANCH_IF_ZERO]:
    "Caso o valor no registrador X seja 0, carrega o valor do operando no registrador PC, efetivamente fazendo com que a instrução nesse índice se torne a próxima a ser executada.",
  [Op.BRANCH_IF_NOT_ZERO]:
    "Caso o valor no registrador X não seja 0, carrega o valor do operando no registrador PC, efetivamente fazendo com que a instrução nesse índice se torne a próxima a ser executada.",
  [Op.INTERRUPT]:
    "Realiza uma `syscall`. Caso o operando seja 0, é invocada a chamada READ. Caso seja 1, é invocada a chamada WRITE.",
};
