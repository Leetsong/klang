#ifndef __KLANG_PARSER_H__
#define __KLANG_PARSER_H__

#include <map>
#include <memory>
#include "lexer.h"
#include "ast.h"

class Parser {

public:
  static std::map<char, int> binops_prio;

public:
  Parser(Lexer &lexer);
  // parse_primary - parses primary
  // primary -> identifierexpr
  //          | numberexpr
  //          | parenexpr
  std::unique_ptr<ExprAST> parse_primary();
  // parse_number_expr - parses numberexpr
  // numberexpr -> numval
  std::unique_ptr<ExprAST> parse_number_expr();
  // parse_paren_expr - parses parenexpr
  // parenexpr -> '(' expression ')'
  std::unique_ptr<ExprAST> parse_paren_expr();
  // parse_identifier_expr - parses identifierexpr
  // identifierexpr -> identifier
  //                 | identifier '(' [expression (, expression)*] ')'
  std::unique_ptr<ExprAST> parse_identifier_expr();
  // parse_expression - parses expression
  // expression -> primary binoprhs
  std::unique_ptr<ExprAST> parse_expression();
  // parse_binoprhs - parses binoprhs
  // binoprhs -> ( binop primary )*
  std::unique_ptr<ExprAST> parse_binoprhs(int expr_prio, std::unique_ptr<ExprAST> rhs);
  // parse_prototype - parses prototype
  // prototype -> identifier '(' [identifier (, identifier)*] ')'
  std::unique_ptr<PrototypeAST> parse_prototype();
  // parse_definition - parses definition
  // definition -> 'def' prototype '{' expression '}'
  std::unique_ptr<FunctionAST> parse_definition();
  // parse_extern - parses external
  // external -> 'extern' prototype
  std::unique_ptr<PrototypeAST> parse_external();
  // parse_toplevelexpr - parses toplevelexpr
  // toplevelexpr -> expression
  std::unique_ptr<FunctionAST> parse_toplevelexpr();
  // parse_top - parses on line of input
  // top -> definition | external | toplevelexpr | ';'
  std::unique_ptr<AST> parse_top();

private:
  // get_binop_prio - get the priority of binop op, or -1
  int get_binop_prio(char op);
  // throw_error
  std::unique_ptr<ExprAST> throw_error(const std::string &message);
  // throw_error_p
  std::unique_ptr<PrototypeAST> throw_error_p(const std::string &message);
  // throw_error_f
  std::unique_ptr<FunctionAST> throw_error_f(const std::string &message);

private:
  Lexer &lexer;

};

#endif