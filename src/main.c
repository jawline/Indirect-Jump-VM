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
    printf("SI%i:%i", machine->stack.data[i].data.idata);
  }
}

void step_machine(struct Machine* machine) {
   uint8_t const_type;
   struct BLValue new_value;

  #ifdef FAST_MODE
    printf("Built with computed goto\n");
    #define GO_NEXT_INSTR() \
     goto *vm_states[machine->program[machine->pc]];
    static void* vm_states[256] = {};
    vm_states[EXIT] = &&dEXIT;
    vm_states[PUSH_CONST] = &&dPUSH_CONST;
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
          BLI32:
	    new_value.data.idata = MEM_IMMEDIATE(int32_t, 1);
            break;
	  default: printf("Invalid type on push\n"); return;
        }
        program_stack_push(&machine->stack, new_value);
        break;
    }
  }

  #undef GO_NEXT_INSTR
}
 
int main(int argc, char** argv) {
  struct Machine m1;
  m1.pc = 0;
  m1.esp = 0;
  m1.ebp = 0;
  program_stack_init(&m1.stack);
  step_machine(&m1); 
  print_state(&m1);
}
