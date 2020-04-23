#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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

void step_machine_slow(struct Machine* machine) {
  while (1) {
    //printf("%li\n", machine->mem[machine->pc]);
    switch (machine->mem[machine->pc]) {
      case 0:
        end_state(machine);
        break;
      case 1:
        MOV_IMMEDIATE("r0", &machine->r0);
        break;
      case 2:
        MOV_IMMEDIATE("r1", &machine->r1);
        break;
      case 3:
        ADD_IMMEDIATE("r0", &machine->r0);
        break;
      case 4:
        ADD_IMMEDIATE("r1", &machine->r1);
        break;
      case 5:
        CMP_IMMEDIATE("r0", &machine->r0);
        break;
      case 6:
        CMP_IMMEDIATE("r1", &machine->r1);
        break;
      case 7:
        jne_imm(machine);
        break;
    }
  }
}

void step_machine_fast(struct Machine* machine) {

  static void* vm_states[] = { &&dend_state, &&dmov_imm_r0, &&dmov_imm_r1, &&dadd_imm_r0, &&dadd_imm_r1, &&dcmp_imm_r0, &&dcmp_imm_r1, &&djne_imm };

  #define GO_NEXT_INSTR() \
    goto *vm_states[machine->mem[machine->pc]]

  GO_NEXT_INSTR();

  dend_state:
    end_state(machine);
    return;
  dmov_imm_r0:
    MOV_IMMEDIATE("r0", &machine->r0);
    GO_NEXT_INSTR();
  dmov_imm_r1:
    MOV_IMMEDIATE("r1", &machine->r1);
    GO_NEXT_INSTR();
  dadd_imm_r0:
    ADD_IMMEDIATE("r0", &machine->r0);
    GO_NEXT_INSTR();
  dadd_imm_r1:
    ADD_IMMEDIATE("r1", &machine->r1);
    GO_NEXT_INSTR();
  dcmp_imm_r0:
    CMP_IMMEDIATE("r0", &machine->r0);
    GO_NEXT_INSTR();
  dcmp_imm_r1:
    CMP_IMMEDIATE("r1", &machine->r1);
    GO_NEXT_INSTR();
  djne_imm:
    jne_imm(machine);
    GO_NEXT_INSTR();
}
 
int main(int argc, char** argv) {
  uint8_t program[] = { 1, 0, 0, 0, 0, 0, 0, 0, 0 /* Mov r0 0 PC 9 */, 2, 0, 0, 0, 0, 0, 0, 0, 0 /* Move r1 0 PC 18 */, 4, 1, 0, 0, 0, 0, 0, 0, 0 /** add r1 1 PC 27 */, 6, 255, 0, 0, 0, 0, 0, 0, 0 /** cmp r1 2^24 PC 36 */, 7, 18, 0, 0, 0, 0, 0, 0, 0 /** Jmp r1 loop */, 3, 1, 0, 0, 0, 0, 0, 0, 0 /** add r0, 1 PC: 54 */, 5, 255, 255, 255, 0, 0, 0, 0, 0 /** cmp r0 */, 7, 9, 0, 0, 0, 0, 0, 0, 0 /** Jmp r0 loop */, 0 };
  struct Machine m1;
  m1.mem = program;
  m1.pc = 0;
  m1.f = 0;
  if (strcmp(argv[1], "slow") == 0) {
    step_machine_slow(&m1);
  } else if (strcmp(argv[1], "fast") == 0) {
    step_machine_fast(&m1);
  }
  print_state(&m1);
}
