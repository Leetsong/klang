#include <llvm/ADT/STLExtras.h>
#include "codegen.h"

#define CODEGEN_RETURN_V(v) do { set_ret_value((v)); return; } while(0)
#define CODEGEN_RETURN_N()  do { set_ret_none(); return; } while(0)

CodeGenerator::CodeGenerator()
  : the_context(),
    the_module(llvm::make_unique<llvm::Module>("sole_module", the_context)),
    ir_builder(the_context),
    named_values() {}

// visit - generates codes for NumberAST
void CodeGenerator::visit(const NumberExprAST &ast) {
  CODEGEN_RETURN_V(llvm::ConstantFP::get(the_context, llvm::APFloat(ast.val)));
}

// visit - generates codes for VariableExprAST
void CodeGenerator::visit(const VariableExprAST &ast) {
  auto v = named_values.find(ast.name);
  if (named_values.end() == v) {
    throw_error_v("unknown variable name");
    CODEGEN_RETURN_N();
  }

  CODEGEN_RETURN_V(v->second);
}

// visit - generates codes for BinaryExprAST
void CodeGenerator::visit(const BinaryExprAST &ast) {
  // generates codes for lhs and rhs
  ast.lhs->accept(*this);
  llvm::Value *l = get_ret_v();
  if (!l) {
    CODEGEN_RETURN_N();
  }

  ast.rhs->accept(*this);
  llvm::Value *r = get_ret_v();
  if (!r) {
    CODEGEN_RETURN_N();
  }

  // generates codes for the binop
  switch (ast.op) {
  case Lexer::operator_lt:
    l = ir_builder.CreateFCmpULT(l, r, "cmptmp");
    CODEGEN_RETURN_V(ir_builder.CreateUIToFP(l, llvm::Type::getDoubleTy(the_context), "booltmp"));
  case Lexer::operator_sub:
    CODEGEN_RETURN_V(ir_builder.CreateFSub(l, r, "subtmp"));
  case Lexer::operator_add:
    CODEGEN_RETURN_V(ir_builder.CreateFAdd(l, r, "addtmp"));
  case Lexer::operator_mul:
    CODEGEN_RETURN_V(ir_builder.CreateFMul(l, r, "multmp"));
  default:
    throw_error_v("invalid binary operator");
    CODEGEN_RETURN_N();
  }
}

void CodeGenerator::visit(const CallExprAST &ast) {
  // look up the name in the global module table
  llvm::Function *callee_ref = the_module->getFunction(ast.callee);
  if (!callee_ref) {
    throw_error_v("unknown function referenced");
    CODEGEN_RETURN_N();
  }

  // if argument mismatch error
  if (callee_ref->arg_size() != ast.args.size()) {
    throw_error_v("incorrect # arguments passed");
    CODEGEN_RETURN_N();
  }

  // generates codes for each arg
  std::vector<llvm::Value *> args;
  for (unsigned i = 0, e = static_cast<unsigned>(ast.args.size()); i < e; i ++) {
    ast.args[i]->accept(*this);
    auto v = get_ret_v();
    if (!v) {
      CODEGEN_RETURN_N();
    } else {
      args.push_back(v);
    }
  }

  // generates codes for call
  CODEGEN_RETURN_V(ir_builder.CreateCall(callee_ref, args, "calltmp"));
}

// visit - generates codes for PrototypeAST
void CodeGenerator::visit(const PrototypeAST &ast) {
  // make type for the args, in Kaleioscope, they are all double
  std::vector<llvm::Type *> arg_types(ast.args.size(), llvm::Type::getDoubleTy(the_context));
  llvm::Type *ret_type = llvm::Type::getDoubleTy(the_context);

  // make type for the function
  llvm::FunctionType *ft = llvm::FunctionType::get(ret_type, arg_types, false);

  // create a function
  llvm::Function *f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, ast.name, the_module.get());

  unsigned idx = 0;
  for (auto &arg : f->args()) {
    arg.setName(ast.args[idx ++]);
  }

  CODEGEN_RETURN_V(f);
}

// visit - generates codes for FunctionAST
void CodeGenerator::visit(const FunctionAST &ast) {
  // check the symbol table
  llvm::Function *f = the_module->getFunction(ast.proto->name);

  if (f && !f->empty()) { // find f, and f is already defined (via "def")
    throw_error_v("function cannot be redefined");
    CODEGEN_RETURN_N();
  } else if (f && f->empty()) { // find f, and f is declared (via "extern")
    unsigned idx = 0;
    for (auto &arg : f->args()) {
      if (ast.proto->args[idx ++] != arg.getName()) {
        throw_error_v("argument name is not the same as the declaration");
        CODEGEN_RETURN_N();
      }
    }
  } else { // f has not already been declared (via "extern") or defined (via "def")
    ast.proto->accept(*this); // we declare it
    if (!(f = get_ret_f())) {
      CODEGEN_RETURN_N();
    }
  }

  // creates the body for f

  // creates a new basic block to start insertion into
  llvm::BasicBlock *bb = llvm::BasicBlock::Create(the_context, "entry", f);
  ir_builder.SetInsertPoint(bb);

  // record the function arguments in the named_values map
  named_values.clear();
  for (auto &arg : f->args()) {
    named_values[arg.getName()] = &arg;
  }

  ast.body->accept(*this);
  if (llvm::Value *ret_value = get_ret_v()) {
    // finish off the function
    ir_builder.CreateRet(ret_value);

    // validate the generated code, checking for consistency
    llvm::verifyFunction(*f);

    CODEGEN_RETURN_V(f);

    return ;
  }

  // error reading body, remove function for redefinition
  f->eraseFromParent();

  CODEGEN_RETURN_N();
}

// get_ret_type - gets the return type
int CodeGenerator::get_ret_type() {
  return ret_type;
}

// get_ret_v - gets the RET_TYPE_VALUEed value ret_v
llvm::Value *CodeGenerator::get_ret_v() {
  return ret_v;
}

// get_ret_f - gets the RET_TYPE_FUNCTIONed value ret_f
llvm::Function *CodeGenerator::get_ret_f() {
  return ret_f;
}

// set_ret_none - set RET_TYPE_NONE when failed
void CodeGenerator::set_ret_none() {
  ret_type = RET_TYPE_NONE;
  ret_f = nullptr; // or ret_v = nullptr
}

// set_ret_value - set RET_TYPE_FUNCTION
void CodeGenerator::set_ret_value(llvm::Function *f) {
  ret_type = RET_TYPE_FUNCTION;
  ret_f = f;
}

// set_ret_value - set RET_TYPE_VALUE
void CodeGenerator::set_ret_value(llvm::Value *v) {
  ret_type = RET_TYPE_VALUE;
  ret_v = v;
}

// throw_error_v
void CodeGenerator::throw_error_v(const std::string &message) {
  std::cerr << message << std::endl;
}
