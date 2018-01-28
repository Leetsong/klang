#include <iostream>
#include <llvm/ADT/STLExtras.h>
#include <llvm/Support/raw_ostream.h>
#include "lexer.h"
#include "parser.h"
#include "codegen.h"

class REPL {
public:
  static REPL *get_instance() {
    if (!instance) {
      instance = new REPL();
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
        default:
          std::cerr << "unknown ret type" << std::endl; break;
        case CodeGenerator::RET_TYPE_NONE: case CodeGenerator::RET_TYPE_VALUE:
          std::cerr << "this will not ever happen!" << std::endl; break;
        case CodeGenerator::RET_TYPE_FUNCTION:
          handle_ret_v(code_gen->get_ret_f()); break;
        }
      }
    }
  }

private:
  void handle_ret_v(llvm::Function *f) {
    std::cout << "read function" << std::endl;
    f->print(llvm::errs());
    std::cerr << std::endl;
  }

private:
  REPL() {
    lexer    = llvm::make_unique<Lexer>(std::cin);
    parser   = llvm::make_unique<Parser>(*lexer);
    code_gen = llvm::make_unique<CodeGenerator>();
  }

private:
  static REPL *instance;

private:
  std::unique_ptr<Lexer>         lexer;
  std::unique_ptr<Parser>        parser;
  std::unique_ptr<CodeGenerator> code_gen;
};

REPL *REPL::instance = nullptr;

int main(int argc, char* argv[]) {
  REPL::get_instance()->run();
  REPL::release();
  return 0;
}