#include "lexer.h"

const std::string Lexer::TOKENS_STR[] = {
  "unknown",
  #define X(a, b) #a ,
  TOKEN_INFO
  #undef X
};

Lexer::Lexer(std::istream &input)
  : input_stream(input) {}

double Lexer::get_curr_numval() const {
  return curr_numval;
}

std::string Lexer::get_curr_identifier() const {
  return curr_identifier;
}

int Lexer::get_curr_token() {
  return curr_token;
}

int Lexer::advance() {
  return curr_token = get_token();
}

int Lexer::get_token() {
  // skip any whitespace.
  while (isspace(last_char)) {
    last_char = input_stream.get();
  }

  // identifier: [a-zA-Z][a-zA-Z0-9]*
  if (isalpha(last_char)) {
    curr_identifier = last_char;
    while(isalnum((last_char = input_stream.get()))) {
      curr_identifier += last_char;
    }

    if (curr_identifier == TOKENS_STR[-token_extern]) {
      return token_extern;
    } else if (curr_identifier == TOKENS_STR[-token_def]) {
      return token_def;
    }

    return token_identifier;
  }

  // number: [0-9.]+, not very correct
  if (isdigit(last_char) || '.' == last_char) {
    std::string number_str;
    do {
      number_str += last_char;
      last_char = input_stream.get();
    } while (isdigit(last_char) || '.' == last_char);

    curr_numval = strtod(number_str.c_str(), NULL);
    return token_numval;
  }

  // comments
  if ('#' == last_char) {
    do {
      last_char = input_stream.get();
    } while (!input_stream.eof() && '\r' != last_char && '\n' != last_char);

    if (!input_stream.eof()) {
      return get_token();
    }
  }

  // eof
  if (input_stream.eof()) {
    return token_eof;
  }

  // for others, we just return themselves, e.g. '+'.
  int this_char = last_char;
  last_char = input_stream.get();

  return this_char;
}