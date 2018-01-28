#ifndef __KLANG_DEF_H__
#define __KLANG_DEF_H__

// X(token, token_id)
#define TOKEN_INFO \
  X(eof,        1) \
  X(def,        2) \
  X(extern,     3) \
  X(identifier, 4) \
  X(numval,     5)

// X(operator, name, operator_priority)
// all priority must be great than or equal to 1
#define OPERATOR_INFO \
  X('<', lt,  10) \
  X('+', add, 20) \
  X('-', sub, 20) \
  X('*', mul, 40)

#endif