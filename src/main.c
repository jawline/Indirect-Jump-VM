#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "opcodes.h"
#include <bllist.h>

DYNAMIC_ARRAY(program_stack, struct BLValue, 1024);

struct Machine {
  uint8_t* program;
  struct program_stack stack;
  uint64_t pc, esp, ebp;
};

#define EQ_BIT 1

#define MEM_IMMEDIATE(type, offset) *(type*)(machine->program + machine->pc + 1 + offset)

void print_state(struct Machine* machine) {
  printf("PC:%li EBP:%li ESP:%li\n", machine->pc, machine->ebp, machine->esp);
  size_t max_size = program_stack_size(&machine->stack);
  for (size_t i = 0; i < max_size; i++) {
    printf("SI%i:%i\n", machine->stack.data[i].data.idata);
  }
}

void add_blvalue(struct BLValue* a, struct BLValue* b) {
  switch (a->type) {
    case BLI32:
      //FIXME: Coerce when other values
      a->data.idata += b->data.idata;
  }
}

void sub_blvalue(struct BLValue* a, struct BLValue* b) {
  switch (a->type) {
    case BLI32:
      //FIXME: Coerce when other values
      a->data.idata -= b->data.idata;
  }
}

void mul_blvalue(struct BLValue* a, struct BLValue* b) {
  switch (a->type) {
    case BLI32:
      //FIXME: Coerce when other values
      a->data.idata *= b->data.idata;
  }
}

void div_blvalue(struct BLValue* a, struct BLValue* b) {
  switch (a->type) {
    case BLI32:
      //FIXME: Coerce when other values
      a->data.idata /= b->data.idata;
  }
}

void step_machine(struct Machine* machine) {
   uint8_t const_type;
   struct BLValue new_value;
   struct BLValue lval;
   struct BLValue rval;

  #ifdef FAST_MODE
    printf("Built with computed goto\n");
    #define GO_NEXT_INSTR() \
     goto *vm_states[machine->program[machine->pc]];
    static void* vm_states[256] = {};
    vm_states[EXIT] = &&dEXIT;
    vm_states[PUSH_CONST] = &&dPUSH_CONST;
    vm_states[ADD] = &&dADD;
    vm_states[SUB] = &&dSUB;
    vm_states[MUL] = &&dMUL;
    vm_states[DIV] = &&dDIV;
  #else
    printf("Built with switch\n");
    #define GO_NEXT_INSTR() //no op
  #endif

  while (1) {
    //printf("%li\n", machine->mem[machine->pc]);
    switch (machine->program[machine->pc]) {
      case EXIT: dEXIT:
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
	printf("Push\n");
        program_stack_push(&machine->stack, new_value);
	GO_NEXT_INSTR();
        break;
      case ADD:
	dADD:
	lval = program_stack_pop(&machine->stack);
	rval = program_stack_pop(&machine->stack);
	add_blvalue(&lval, &rval);
	program_stack_push(&machine->stack, lval);
	machine->pc++;
	GO_NEXT_INSTR();
	break;
      case SUB:
	dSUB:
	lval = program_stack_pop(&machine->stack);
	rval = program_stack_pop(&machine->stack);
	sub_blvalue(&lval, &rval);
	program_stack_push(&machine->stack, lval);
	machine->pc++;
	GO_NEXT_INSTR();
	break;
      case DIV:
	dDIV:
	lval = program_stack_pop(&machine->stack);
	rval = program_stack_pop(&machine->stack);
	div_blvalue(&lval, &rval);
	program_stack_push(&machine->stack, lval);
	machine->pc++;
	GO_NEXT_INSTR();
	break;
      case MUL:
	dMUL:
	lval = program_stack_pop(&machine->stack);
	rval = program_stack_pop(&machine->stack);
	mul_blvalue(&lval, &rval);
	program_stack_push(&machine->stack, lval);
	machine->pc++;
	GO_NEXT_INSTR();
	break;
    }
  }

  #undef GO_NEXT_INSTR
}

uint8_t* read_entire_program(char* file) {
  printf("Loading %s\n", file);
  FILE* fp = fopen(file, "r");
  fseek(fp, 0, SEEK_END);
  size_t end = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  uint8_t* data = (uint8_t*) malloc(end);
  fread(data, end, 1, fp);
  printf("Loaded\n");
  return data;
}
 
int main(int argc, char** argv) {
  struct Machine m1;
  m1.program = read_entire_program(argv[1]);
  m1.pc = 0;
  m1.esp = 0;
  m1.ebp = 0;
  program_stack_init(&m1.stack);
  step_machine(&m1); 
  print_state(&m1);
}
