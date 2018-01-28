#ifndef __KLANG_AST_H__
#define __KLANG_AST_H__

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "lexer.h"

class Visitor;

class AST {
public:
  virtual ~AST() {}
  // accept - accept accepts a visit to visit it
  virtual void accept(Visitor &v) = 0;
};

// ExprAST - Base class for all expression nodes.
class ExprAST : public AST {
public:
  virtual ~ExprAST() {}
};

// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST {
public:
  NumberExprAST(double v);
  // accept - accept accepts a visit to visit it
  void accept(Visitor &v) override;

public:
  double val;
};

// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST {
public:
  VariableExprAST(const std::string &name);
  // accept - accept accepts a visit to visit it
  void accept(Visitor &v) override;

public:
  std::string name;
};

// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST {
public:
  BinaryExprAST(char op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs);
  // accept - accept accepts a visit to visit it
  void accept(Visitor &v) override;

public:
  char                     op;
  std::unique_ptr<ExprAST> lhs, rhs;
};

// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {
public:
  CallExprAST(const std::string &callee, std::vector<std::unique_ptr<ExprAST>> args);
  // accept - accept accepts a visit to visit it
  void accept(Visitor &v) override;

public:
  std::string                           callee;
  std::vector<std::unique_ptr<ExprAST>> args;
};

// PrototypeAST - This class represents the "prototype" for a function
// which captures its name, and its argument names (thus implicitly the number
// of arguments the funtion takes)
class PrototypeAST : public AST {
public:
  PrototypeAST(const std::string &name, const std::vector<std::string> &args);
  // accept - accept accepts a visit to visit it
  void accept(Visitor &v) override;

public:
  std::string              name;
  std::vector<std::string> args;
};

// FunctionAST - This class represents a function definition itself.
class FunctionAST : public AST {
public:
  FunctionAST(std::unique_ptr<PrototypeAST> proto, std::unique_ptr<ExprAST> body);
  // accept - accept accepts a visit to visit it
  void accept(Visitor &v) override;

public:
  std::unique_ptr<PrototypeAST> proto;
  std::unique_ptr<ExprAST>      body;
};

// Visitor - a visitor that can visit AST
class Visitor {
public:
  virtual ~Visitor() {}
  // visit - visits AST
  virtual void visit(const AST &ast)             = 0;
  // visit - visits ExprAST
  virtual void visit(const ExprAST &ast)         = 0;
  // visit - visits NumberExprAST
  virtual void visit(const NumberExprAST &ast)   = 0;
  // visit - visits VariableExprAST
  virtual void visit(const VariableExprAST &ast) = 0;
  // visit - visits BinaryExprAST
  virtual void visit(const BinaryExprAST &ast)   = 0;
  // visit - visits CallExprAST
  virtual void visit(const CallExprAST &ast)     = 0;
  // visit - visits PrototypeAST
  virtual void visit(const PrototypeAST &ast)    = 0;
  // visit - visits FunctionAST
  virtual void visit(const FunctionAST &ast)     = 0;
};

#endif