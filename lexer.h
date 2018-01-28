#ifndef __KLANG_LEXER_H__
#define __KLANG_LEXER_H__

#include <cstdio>
#include <cctype>
#include <istream>
#include <iostream>
#include <string>
#include "def.h"

class Lexer {

public:
  // this enumeration stores all tokens
  // they must be negative, so that their prio is
  // less than all binop
  enum {
    #define X(a, b) token_##a = -b,
    TOKEN_INFO
    #undef X
  };

  enum {
    #define X(a, b, c) operator_##b = a,
    OPERATOR_INFO
    #undef X
  };

public:
  // tokens str contains all tokens in string
  static const std::string TOKENS_STR[];

public:
  // a Lexer takes an input stream and parse it
  Lexer(std::istream &input);
  // get_curr_numval - returns curr_numval
  double get_curr_numval() const;
  // get_curr_identifier - returns curr_identifier
  std::string get_curr_identifier() const;
  // advance - make the lexer advance one step: recognize next token
  int advance();
  // get_curr_token - gets the current token that the lexer recognized just now
  int get_curr_token();

private:
  // get_token - Return the next token from standard input
  int get_token();

private:
  std::istream &input_stream;
  int           last_char       = ' '; // initialized to space so that advance can make it read
  std::string   curr_identifier = "";  // filled in if tok_identifier
  double        curr_numval     = 0;   // filled in if tok_number
  int           curr_token      = ';'; // curr_token stores the token recognized just now

};

#endif