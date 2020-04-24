#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "opcodes.h"

struct Machine {
  uint8_t* mem;
  uint64_t pc, r0, r1, f;
};

#define EQ_BIT 1

#define MEM_IMMEDIATE_64() *(uint64_t*)(machine->mem + machine->pc + 1)

#define end_state(machine) \
  printf("Finished"); \
  return; \

#define MOV_IMMEDIATE(regid, reg) \
  *reg = MEM_IMMEDIATE_64(); \
  machine->pc += 9; \
  //printf("movi %s %li\n", regid, *reg);

#define mov_imm_r0(machine) MOV_IMMEDIATE("r0", &machine->r0) 

#define ADD_IMMEDIATE(regid, reg) \
  *reg += MEM_IMMEDIATE_64(); \
  machine->pc += 9; \
  //printf("addi %s, %li\n", regid, *reg);
#define add_imm_r0(machine) ADD_IMMEDIATE("r0", &machine->r0)

#define CMP_IMMEDIATE(regid, reg) \
  if (*reg == MEM_IMMEDIATE_64()) { \
    machine->f |= EQ_BIT; \
  } else { \
    machine->f &= !EQ_BIT; \
  } \
  machine->pc += sizeof(uint64_t) + 1; \

#define cmp_imm_r0(machine) CMP_IMMEDIATE("r0", &machine->r0);

#define jne_imm(machine) \
  if (machine->f & EQ_BIT) { \
    machine->pc += sizeof(uint64_t) + 1; \
  } else { \
    machine->pc = MEM_IMMEDIATE_64(); \
  }

void print_state(struct Machine* machine) {
  printf("%li %li %li\n", machine->pc, machine->r0, machine->r1);
}

void step_machine(struct Machine* machine) {

  #ifdef FAST_MODE
    printf("Built with computed goto\n");
    #define GO_NEXT_INSTR() \
     goto *vm_states[machine->mem[machine->pc]];
    static void* vm_states[256] = {};
    vm_states[EXIT] = &&dEXIT;
  #else
    printf("Built with switch\n");
    #define GO_NEXT_INSTR() //no op
  #endif

  while (1) {
    //printf("%li\n", machine->mem[machine->pc]);
    switch (machine->mem[machine->pc]) {
      case EXIT:
        dEXIT:
        end_state(machine);
        GO_NEXT_INSTR()
        break;
    }
  }

  #undef GO_NEXT_INSTR
}
 
int main(int argc, char** argv) {
  struct Machine m1;
  m1.pc = 0;
  m1.f = 0;
  step_machine(&m1); 
  print_state(&m1);
}
