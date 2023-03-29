%top{
  #include "parser/literals.h"
  #include "parser/syntax/parser.hpp"
  #include "parser/syntax/location.hpp"

  enum class keyword { TRUE, FALSE, IF, ELSE, WHILE, RETURN, GOTO, WRITE, READ };
}

%class {
private:
  yy::parser::symbol_type make_keyword(keyword k, yy::location loc);

public:
  std::map<std::string, keyword> keyword_map;

  void init_default_keywords();
  yy::parser::symbol_type look_for_keyword(std::string identifier, yy::location loc);
}

%option bison-complete
%option bison-cc-namespace=yy
%option bison-cc-parser=parser
%option bison-locations
%option reentrant

%option freespace
%option unicode

%option namespace=yy
%option lexer=scanner

%option fast
%option flex

/* TODO: UAX 31 maybe? */
identifier   [A-Za-z_][A-Za-z0-9_]*

integer      [0-9]+

exponent     [Ee][-+]?[0-9]+
float        [0-9]*([0-9]|\.[0-9]|[0-9]\.)[0-9]*{exponent}?

/* TODO: Char = Grapheme */
char         \' ( \\. | [^\\'\n] )* \'
string       \" ( \\. | [^\\"\n] )* \"

%%
\s               // whitespace
"//".*           // inline comment
"/*"(.|\n)*?"*/" // multiline comment

{identifier}     { return look_for_keyword(str(), location()); }
{integer}        { return yy::parser::make_INT_LITERAL(parse_int(str()), location()); }
{float}          { return yy::parser::make_FLOAT_LITERAL(parse_float(str()), location()); }
{char}           { return yy::parser::make_CHAR_LITERAL(parse_char(str()), location()); }
{string}         { return yy::parser::make_STRING_LITERAL(parse_string(str()), location()); }
"+="             { return yy::parser::make_PLUS_ASSIGN(location()); }
"-="             { return yy::parser::make_MINUS_ASSIGN(location()); }
"*="             { return yy::parser::make_STAR_ASSIGN(location()); }
"/="             { return yy::parser::make_SLASH_ASSIGN(location()); }
"%="             { return yy::parser::make_PERCENT_ASSIGN(location()); }
">="             { return yy::parser::make_GTEQ(location()); }
"<="             { return yy::parser::make_LTEQ(location()); }
"=="             { return yy::parser::make_EQUALS(location()); }
"!="             { return yy::parser::make_NEQUALS(location()); }
"&&"             { return yy::parser::make_AND(location()); }
"||"             { return yy::parser::make_OR(location()); }
"!"              { return yy::parser::make_NOT(location()); }
"="              { return yy::parser::make_ASSIGN(location()); }
"+"              { return yy::parser::make_PLUS(location()); }
"-"              { return yy::parser::make_MINUS(location()); }
"*"              { return yy::parser::make_STAR(location()); }
"/"              { return yy::parser::make_SLASH(location()); }
"<"              { return yy::parser::make_LT(location()); }
">"              { return yy::parser::make_GT(location()); }
"%"              { return yy::parser::make_PERCENT(location()); }
"("              { return yy::parser::make_LPARENS(location()); }
")"              { return yy::parser::make_RPARENS(location()); }
"{"              { return yy::parser::make_LCURLY(location()); }
"}"              { return yy::parser::make_RCURLY(location()); }
";"              { return yy::parser::make_SEMI(location()); }
":"              { return yy::parser::make_COLON(location()); }
","              { return yy::parser::make_COMMA(location()); }
<<EOF>>          { return yy::parser::make_YYEOF(location()); }
.                { return yy::parser::make_YYUNDEF(location()); }
%%

void yy::scanner::init_default_keywords() {
  this->keyword_map["if"] = keyword::IF;
  this->keyword_map["else"] = keyword::ELSE;
  this->keyword_map["while"] = keyword::WHILE;
  this->keyword_map["return"] = keyword::RETURN;
  this->keyword_map["goto"] = keyword::GOTO;
  this->keyword_map["write"] = keyword::WRITE;
  this->keyword_map["read"] = keyword::READ;

  this->keyword_map["true"] = keyword::TRUE;
  this->keyword_map["false"] = keyword::FALSE;
}

yy::parser::symbol_type yy::scanner::make_keyword(keyword k, yy::location loc) {
  switch(k) {
    case keyword::TRUE:
      return yy::parser::make_BOOLEAN_LITERAL(true, loc);
    case keyword::FALSE:
      return yy::parser::make_BOOLEAN_LITERAL(false, loc);
    case keyword::IF:
      return yy::parser::make_IF(loc);
    case keyword::ELSE:
      return yy::parser::make_ELSE(loc);
    case keyword::WHILE:
      return yy::parser::make_WHILE(loc);
    case keyword::RETURN:
      return yy::parser::make_RETURN(loc);
    case keyword::GOTO:
      return yy::parser::make_GOTO(loc);
    case keyword::WRITE:
      return yy::parser::make_WRITE(loc);
    case keyword::READ:
      return yy::parser::make_READ(loc);
    default:
      return yy::parser::make_YYerror(loc);
  }
}

yy::parser::symbol_type yy::scanner::look_for_keyword(std::string identifier, yy::location loc) {
  auto found = this->keyword_map.find(identifier);

  if (found == this->keyword_map.end()) {
    return yy::parser::make_IDENTIFIER(identifier, loc);
  } else {
    auto pair = *found;
    auto keyword = pair.second;
    return make_keyword(keyword, loc);
  }
}
