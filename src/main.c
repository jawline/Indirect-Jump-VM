#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "opcodes.h"
#include <bllist.h>
#include <stdbool.h>

DYNAMIC_ARRAY(program_stack, struct BLValue, 1024);

struct Machine {
  uint8_t* program;
  struct program_stack stack;
  uint64_t pc, ebp;
};

#define MEM_IMMEDIATE(type, offset) *(type*)(machine->program + machine->pc + 1 + offset)

void print_state(struct Machine* machine) {
  size_t max_size = program_stack_size(&machine->stack);
  printf("PC:%lu EBP:%lu ESP:%li\n", machine->pc, machine->ebp, max_size);
  for (size_t i = 0; i < max_size; i++) {
    printf("SI%zu:%i\n", i,machine->stack.data[i].data.idata);
  }
}

inline void add_blvalue(struct BLValue* a, struct BLValue* b) __attribute__((always_inline)); 
void add_blvalue(struct BLValue* a, struct BLValue* b) {
  switch (a->type) {
    case BLI32:
      //FIXME: Coerce when other values
      a->data.idata += b->data.idata;
  }
}

inline void sub_blvalue(struct BLValue* a, struct BLValue* b) __attribute__((always_inline));
void sub_blvalue(struct BLValue* a, struct BLValue* b) {
  switch (a->type) {
    case BLI32:
      //FIXME: Coerce when other values
      a->data.idata -= b->data.idata;
  }
}

inline void mul_blvalue(struct BLValue* a, struct BLValue* b) __attribute__((always_inline));
void mul_blvalue(struct BLValue* a, struct BLValue* b) {
  switch (a->type) {
    case BLI32:
      //FIXME: Coerce when other values
      a->data.idata *= b->data.idata;
  }
}

inline void div_blvalue(struct BLValue* a, struct BLValue* b) __attribute__((always_inline));
void div_blvalue(struct BLValue* a, struct BLValue* b) {
  switch (a->type) {
    case BLI32:
      //FIXME: Coerce when other values
      a->data.idata /= b->data.idata;
  }
}

inline bool blvalue_eq(struct BLValue* a, struct BLValue* b) __attribute__((always_inline));
bool blvalue_eq(struct BLValue* a, struct BLValue* b) {
  return a->data.idata == b->data.idata;
}

void step_machine(struct Machine* machine) {
   uint8_t const_type;
   int32_t offset;
   struct BLValue new_value;
   struct BLValue lval;
   struct BLValue rval;

  #ifdef FAST_MODE
    printf("Built with computed goto\n");
    #define GO_NEXT_INSTR() \
     goto *vm_states[machine->program[machine->pc]];
    static void* vm_states[255] = {};
    vm_states[EXIT] = &&dEXIT;
    vm_states[PUSH_CONST] = &&dPUSH_CONST;
    vm_states[PUSH_RELATIVE_SP] = &&dPUSH_RELATIVE_SP;
    vm_states[POP] = &&dPOP;
    vm_states[ADD] = &&dADD;
    vm_states[SUB] = &&dSUB;
    vm_states[MUL] = &&dMUL;
    vm_states[DIV] = &&dDIV;
    vm_states[JMP] = &&dJMP;
    vm_states[JE] = &&dJE;
    vm_states[JNE] = &&dJNE;
  #else
    printf("Built with switch\n");
    #define GO_NEXT_INSTR() //no op
  #endif

  while (1) {
    switch (machine->program[machine->pc]) {
      case EXIT:
        dEXIT:
        printf("Bye. Have a good time\n");
        return;
      case PUSH_CONST:
        dPUSH_CONST:
         const_type = MEM_IMMEDIATE(uint8_t, 0);
        new_value.type = const_type;
        switch (const_type) {
          case BLI32:
            new_value.data.idata = MEM_IMMEDIATE(int32_t, 1);
            machine->pc += sizeof(int32_t) + 2;
            break;
          default:
            printf("Invalid type on push %i (%i)\n", const_type, BLI32);
            return;
        }
        program_stack_push(&machine->stack, new_value);
        GO_NEXT_INSTR();
        break;
      case PUSH_RELATIVE_SP:
        dPUSH_RELATIVE_SP:
        offset = MEM_IMMEDIATE(int32_t, 0);
        size_t pos = program_stack_size(&machine->stack) + offset;
        program_stack_push(&machine->stack, machine->stack.data[pos]);
        machine->pc += 5;
        GO_NEXT_INSTR();
        break;
      case POP:
        dPOP:
        program_stack_pop(&machine->stack);
        machine->pc++;
        GO_NEXT_INSTR();
        break;
      case ADD:
        dADD:
        rval = program_stack_pop(&machine->stack);
        lval = program_stack_pop(&machine->stack);
        add_blvalue(&lval, &rval);
        program_stack_push(&machine->stack, lval);
        machine->pc++;
        GO_NEXT_INSTR();
        break;
      case SUB:
        dSUB:
        rval = program_stack_pop(&machine->stack);
        lval = program_stack_pop(&machine->stack);
        sub_blvalue(&lval, &rval);
        program_stack_push(&machine->stack, lval);
        machine->pc++;
        GO_NEXT_INSTR();
        break;
      case DIV:
        dDIV:
        rval = program_stack_pop(&machine->stack);
        lval = program_stack_pop(&machine->stack);
        div_blvalue(&lval, &rval);
        program_stack_push(&machine->stack, lval);
        machine->pc++;
        GO_NEXT_INSTR();
        break;
      case MUL:
        dMUL:
        rval = program_stack_pop(&machine->stack);
        lval = program_stack_pop(&machine->stack);
        mul_blvalue(&lval, &rval);
        program_stack_push(&machine->stack, lval);
        machine->pc++;
        GO_NEXT_INSTR();
        break;
      case JMP:
        dJMP:
        machine->pc = MEM_IMMEDIATE(uint32_t, 0);
        GO_NEXT_INSTR();
        break;
      case JE:
        dJE:
        lval = program_stack_pop(&machine->stack);
        rval = program_stack_pop(&machine->stack);
        if (blvalue_eq(&lval, &rval)) {
          machine->pc = MEM_IMMEDIATE(uint32_t, 0);
        } else {
          machine->pc += 5;
        }
        GO_NEXT_INSTR();
        break;
      case JNE:
        dJNE:
        lval = program_stack_pop(&machine->stack);
        rval = program_stack_pop(&machine->stack);
        if (!blvalue_eq(&lval, &rval)) {
          machine->pc = MEM_IMMEDIATE(uint32_t, 0);
        } else {
          machine->pc += 5;
        }
        GO_NEXT_INSTR();
        break;
    }
  }

  #undef GO_NEXT_INSTR
}

uint8_t* read_entire_program(char* file) {
  printf("Loading %s\n", file);
  FILE* fp = fopen(file, "r");
  
  if (!fp) {
    return NULL;
  }

  fseek(fp, 0, SEEK_END);
  size_t end = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  uint8_t* data = (uint8_t*) malloc(end);
  if (fread(data, end, 1, fp) != 1) {
    free(data);
    return NULL;
  }

  printf("Loaded\n");
  return data;
}
 
int main(int argc, char** argv) {
  struct Machine m1;
  m1.program = read_entire_program(argv[1]);
  m1.pc = 0;
  m1.ebp = 0;
  program_stack_init(&m1.stack);
  step_machine(&m1); 
  printf("Final State\n");
  print_state(&m1);
}
