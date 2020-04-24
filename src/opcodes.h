#ifndef _OPCODES_H_
#define _OPCODES_H_
#include <stdint.h>

enum BLOP {
  EXIT = 0,
  PUSH_CONST,
  ADD,
  SUB,
  MUL,
  DIV 
};

enum BLType {
  BLI32 = 0 
};

union BLData {
  int32_t idata;
};

struct BLValue {
  enum BLType type;
  union BLData data;  
};

#endif 
