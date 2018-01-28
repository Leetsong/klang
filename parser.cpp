#include <llvm/ADT/STLExtras.h>
#include "parser.h"

std::map<char, int> Parser::binops_prio = {
  #define X(a, b, c) {a, c},
  OPERATOR_INFO
  #undef X
};

Parser::Parser(Lexer &lexer) : lexer(lexer) {}

// parse_primary - parses primary
// primary -> identifierexpr
//          | numberexpr
//          | parenexpr
std::unique_ptr<ExprAST> Parser::parse_primary() {
  switch(lexer.get_curr_token()) {
  case Lexer::token_identifier: return parse_identifier_expr();
  case Lexer::token_numval:     return parse_number_expr();
  case '(':              return parse_paren_expr();
  default: return throw_error("unknown token when expecting an expression");
  }
}

// parse_number_expr - parses numberexpr
// numberexpr -> numval
std::unique_ptr<ExprAST> Parser::parse_number_expr() {
  std::unique_ptr<ExprAST> e = llvm::make_unique<NumberExprAST>(lexer.get_curr_numval());
  lexer.advance(); // eat numval
  return e;
}

// parse_paren_expr - parses parenexpr
// parenexpr -> '(' expression ')'
std::unique_ptr<ExprAST> Parser::parse_paren_expr() {
  lexer.advance(); // eat '('

  auto e = parse_expression();
  if (!e) { return nullptr; }

  if (')' != lexer.get_curr_token()) {
    return throw_error("expected ')'");
  }

  lexer.advance(); // eat ')'

  return e;
}

// parse_identifier_expr - parses identifierexpr
// identifierexpr -> identifier
//                 | identifier '(' [expression (, expression)*] ')'
std::unique_ptr<ExprAST> Parser::parse_identifier_expr() {
  std::string id = lexer.get_curr_identifier();

  lexer.advance(); // eat identifier

  // an identifier
  if ('(' != lexer.get_curr_token()) {
    return llvm::make_unique<VariableExprAST>(id);
  }

  // a function invoking
  lexer.advance();

  std::vector<std::unique_ptr<ExprAST>> args;
  while (1) {
    auto e = parse_expression();
    if (!e) { return nullptr; }
    else { args.push_back(std::move(e)); }

    if (')' == lexer.get_curr_token()) {
      break;
    } else if (',' != lexer.get_curr_token()) {
      return throw_error("expected ',' or ')' in argument list");
    } else {
      lexer.advance(); // eat ','
    }
  }

  lexer.advance(); // eat ')'

  return llvm::make_unique<CallExprAST>(id, std::move(args));
}

// parse_expression - parses expression
// expression -> primary binoprhs
std::unique_ptr<ExprAST> Parser::parse_expression() {
  auto lhs = parse_primary();
  if (!lhs) { return nullptr; }

  return parse_binoprhs(1, std::move(lhs)); // 1 is the base prio for binop
}

// parse_binoprhs - parses binoprhs
// binoprhs -> ( binop primary )*
std::unique_ptr<ExprAST> Parser::parse_binoprhs(int expr_prio, std::unique_ptr<ExprAST> lhs) {
  // if this is a binop, find its priority
  while (1) {
    int prio = get_binop_prio((char) lexer.get_curr_token());

    // if this is a binop that binds at least as tightly as the current binop,
    // eat it, otherwise we are done.
    if (prio < expr_prio) {
      return lhs;
    }

    int binop = lexer.get_curr_token();
    lexer.advance(); // eat binop

    // parse the primary expression afer the binop
    auto rhs = parse_primary();
    if (!rhs) { return nullptr; }

    // if binop binds less tightly with rhs than the operator after rhs, let
    // the pending operator take rhs as its lhs
    int next_binop_prio = get_binop_prio((char) lexer.get_curr_token());
    if (prio < next_binop_prio) {
      rhs = parse_binoprhs(prio + 1, std::move(rhs));
      if (!rhs) { return nullptr; }
    }

    // merge lhs/rhs to a binary expression
    lhs = llvm::make_unique<BinaryExprAST>((char) binop, std::move(lhs), std::move(rhs));
  }
}

// parse_prototype - parses prototype
// prototype -> identifier '(' [identifier (, identifier)*] ')'
std::unique_ptr<PrototypeAST> Parser::parse_prototype() {
  if (lexer.get_curr_token() != Lexer::token_identifier) {
    return throw_error_p("expected function name in prototype");
  }

  std::string fname = lexer.get_curr_identifier();
  lexer.advance(); // eat identifier

  if ('(' != lexer.get_curr_token()) {
    return throw_error_p("expected '(' in prototype");
  }
  lexer.advance(); // eat '('

  std::vector<std::string> args;
  while (1) {
    if (lexer.get_curr_token() != Lexer::token_identifier) {
      return throw_error_p("expected function name in prototype");
    }

    args.push_back(lexer.get_curr_identifier());
    lexer.advance(); // eat identifier

    if (')' == lexer.get_curr_token()) {
      break;
    } else if (',' != lexer.get_curr_token()) {
      return throw_error_p("expected ')' or ',' in prototype");
    } else {
      lexer.advance(); // eat ','
    }
  }
  lexer.advance(); // eat ')'

  return llvm::make_unique<PrototypeAST>(fname, args);
}

// parse_definition - parses definition
// definition -> 'def' prototype '{' expression '}'
std::unique_ptr<FunctionAST> Parser::parse_definition() {
  lexer.advance(); // eat 'def'

  auto proto = parse_prototype();
  if (!proto) { return nullptr; }

  if ('{' != lexer.get_curr_token()) {
    return throw_error_f("expected '{' in function body");
  }
  lexer.advance(); // eat '{'

  auto e = parse_expression();
  if (!e) { return nullptr; }

  if ('}' != lexer.get_curr_token()) {
    return throw_error_f("expected '{' in function body");
  }
  lexer.advance();

  return llvm::make_unique<FunctionAST>(std::move(proto), std::move(e));
}

// parse_extern - parses external
// external -> 'extern' prototype
std::unique_ptr<PrototypeAST> Parser::parse_external() {
  lexer.advance(); // eat 'extern'

  auto proto = parse_prototype();
  if (!proto) { return nullptr; }

  return proto;
}

// parse_toplevelexpr - parses toplevelexpr
// toplevelexpr -> expression
std::unique_ptr<FunctionAST> Parser::parse_toplevelexpr() {
  auto e = parse_expression();
  if (!e) { return nullptr; }

  auto proto = llvm::make_unique<PrototypeAST>("", std::vector<std::string>());
  return llvm::make_unique<FunctionAST>(std::move(proto), std::move(e));
}

// parse_top - parses on line of input
// top -> definition | external | toplevelexpr | ';'
std::unique_ptr<AST> Parser::parse_top() {
  switch (lexer.get_curr_token()) {
  case ';':                 return nullptr;
  case Lexer::token_def:    return parse_definition();
  case Lexer::token_extern: return parse_external();
  default:                  return parse_toplevelexpr();
  }
}

// get_binop_prio - get the priority of binop op, or -1
int Parser::get_binop_prio(char op) {
  if (!isascii(op)) { return -1; }

  int prio = binops_prio[op];
  // priority of each operator is 1 at least
  if (prio <= 0) { return -1; }

  return prio;
}

// throw_error
std::unique_ptr<ExprAST> Parser::throw_error(const std::string &message) {
  std::cerr << message << std::endl;
  return nullptr;
}

// throw_error_p
std::unique_ptr<PrototypeAST> Parser::throw_error_p(const std::string &message) {
  throw_error(message);
  return nullptr;
}

// throw_error_f
std::unique_ptr<FunctionAST> Parser::throw_error_f(const std::string &message) {
  throw_error(message);
  return nullptr;
}