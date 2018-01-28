#include <iostream>
#include <llvm/ADT/STLExtras.h>
#include <llvm/Support/raw_ostream.h>
#include "lexer.h"
#include "parser.h"
#include "codegen.h"

class Driver {
public:
  static Driver *get_instance() {
    if (!instance) {
      instance = new Driver();
    }

    return instance;
  }

  static void release() {
    delete instance;
    instance = nullptr;
  }

public:
  void run() {
    while (1) {
      std::cout << "> " << std::flush;
      lexer->advance();
      if (lexer->get_curr_token() == Lexer::token_eof) {
        return;
      } else {
        std::unique_ptr<AST> ast;
        do {
          ast = parser->parse_top();
        } while(!ast && lexer->advance());
        ast->accept(*code_gen);
        switch (code_gen->get_ret_type()) {
        default: std::cerr << "unknown ret type" << std::endl; break;
        case CodeGenerator::RET_TYPE_NONE: break;
        case CodeGenerator::RET_TYPE_VALUE: handle_ret_v(code_gen->get_ret_v()); break;
        case CodeGenerator::RET_TYPE_FUNCTION: handle_ret_v(code_gen->get_ret_f()); break;
        }
      }
    }
  }

private:
  void handle_ret_v(llvm::Value *v) {
    std::cout << "Read external" << std::endl;
    v->print(llvm::errs());
    std::cerr << std::endl;
  }

  void handle_ret_v(llvm::Function *f) {
    std::cout << "Read definition" << std::endl;
    f->print(llvm::errs());
    std::cerr << std::endl;
  }

private:
  Driver() {
    lexer    = llvm::make_unique<Lexer>(std::cin);
    parser   = llvm::make_unique<Parser>(*lexer);
    code_gen = llvm::make_unique<CodeGenerator>();
  }

private:
  static Driver *instance;

private:
  std::unique_ptr<Lexer>         lexer;
  std::unique_ptr<Parser>        parser;
  std::unique_ptr<CodeGenerator> code_gen;
};

Driver *Driver::instance = nullptr;

int main(int argc, char* argv[]) {
  Driver::get_instance()->run();
  Driver::release();
  return 0;
}