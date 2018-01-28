#include "ast.h"

NumberExprAST::NumberExprAST(double v)
    : val(v) {}

// accept - accept accepts a visit to visit NumberExprAST
void NumberExprAST::accept(Visitor &v) {
  v.visit(*this);
}

VariableExprAST::VariableExprAST(const std::string &name)
    : name(name) {}

// accept - accept accepts a visit to visit VariableExprAST
void VariableExprAST::accept(Visitor &v) {
  v.visit(*this);
}

BinaryExprAST::BinaryExprAST(char op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs)
    : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

// accept - accept accepts a visit to visit BinaryExprAST
void BinaryExprAST::accept(Visitor &v) {
  v.visit(*this);
}

CallExprAST::CallExprAST(const std::string &callee, std::vector<std::unique_ptr<ExprAST>> args)
    : callee(callee), args(std::move(args)) {}

// accept - accept accepts a visit to visit CallExprAST
void CallExprAST::accept(Visitor &v) {
  v.visit(*this);
}

PrototypeAST::PrototypeAST(const std::string &name, const std::vector<std::string> &args)
    : name(name), args(args) {}

// accept - accept accepts a visit to visit PrototypeAST
void PrototypeAST::accept(Visitor &v) {
  v.visit(*this);
}

FunctionAST::FunctionAST(std::unique_ptr<PrototypeAST> proto, std::unique_ptr<ExprAST> body)
    : proto(std::move(proto)), body(std::move(body)) {}

// accept - accept accepts a visit to visit FunctionAST
void FunctionAST::accept(Visitor &v) {
  v.visit(*this);
}