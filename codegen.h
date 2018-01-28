#ifndef __KLANG_CODEGEN_H__
#define __KLANG_CODEGEN_H__

#include "ast.h"
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>

// CodeGenerator - CodeGenerator is a visitor that can generate codes
class CodeGenerator : public Visitor {
public:
  enum { RET_TYPE_NONE = -1, RET_TYPE_FUNCTION = 0, RET_TYPE_VALUE = 1};

public:
  CodeGenerator();
  // visit - visits AST
  void visit(const AST &ast) override;
  // visit - visits ExprAST
  void visit(const ExprAST &ast) override;
  // visit - generates codes for NumberAST
  void visit(const NumberExprAST &ast) override;
  // visit - generates codes for VariableExprAST
  void visit(const VariableExprAST &ast) override;
  // visit - generates codes for BinaryExprAST
  void visit(const BinaryExprAST &ast) override;
  // visit - generates codes for CallExprAST
  void visit(const CallExprAST &ast) override;
  // visit - generates codes for PrototypeAST
  void visit(const PrototypeAST &ast) override;
  // visit - generates codes for FunctionAST
  void visit(const FunctionAST &ast) override;
  // get_ret_type - gets the return type
  int get_ret_type();
  // get_ret_v - gets the RET_TYPE_VALUEed value ret_v
  llvm::Value *get_ret_v();
  // get_ret_f - gets the RET_TYPE_FUNCTIONed value ret_f
  llvm::Function *get_ret_f();

private:
  // set_ret_none - set RET_TYPE_NONE when failed
  void set_ret_none();
  // set_ret_value - set RET_TYPE_FUNCTION
  void set_ret_value(llvm::Function *f);
  // set_ret_value - set RET_TYPE_VALUE
  void set_ret_value(llvm::Value *v);
  // throw_error_v
  void throw_error_v(const std::string &message);

private:
  llvm::LLVMContext                     the_context;
  std::unique_ptr<llvm::Module>         the_module;
  llvm::IRBuilder<>                     ir_builder;
  std::map<std::string, llvm::Value *>  named_values;

  int ret_type;
  union {
    llvm::Function *ret_f;
    llvm::Value    *ret_v;
  };
};

#endif
