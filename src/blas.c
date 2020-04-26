#include <bllist.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include "opcodes.h"

DYNAMIC_ARRAY(program, uint8_t, 1024);
uint8_t* program_expand(struct program* p, size_t bytes) {
  size_t start = program_size(p);
  for (size_t i = 0; i < bytes; i++) {
    program_push(p, 0);
  }
  return p->data + start;
}

enum LexType {
  CMD,
  VAL,
  COMMA
};


union ValData {
  int32_t idata;
};

struct LexItem {
  enum LexType type;
  char lbuffer[512];
  struct BLValue val;
};

struct Label {
  char name[512];
  size_t pos;
};

DYNAMIC_ARRAY(labels, struct Label, 8);

int peekc(FILE* rdr) {
  int c = fgetc(rdr);
  ungetc(c, rdr);
  return c;
}

void read_until_break(FILE* rdr, char* lbuffer) {
  char nextc = peekc(rdr);
  while (nextc != EOF && nextc != ':' && nextc != ',' && nextc != ' ' && nextc != '\n') {
    *lbuffer = fgetc(rdr);
    nextc = peekc(rdr);
    lbuffer++;
  } 
  *lbuffer = 0; 
}

bool lex_next(FILE* rdr, struct LexItem* result) {
  char* lbuffer = result->lbuffer;

  lbuffer[0] = fgetc(rdr);

  if (feof(rdr)) {
    return false;
  }

  if (lbuffer[0] == ',') {
    result->type == COMMA;
  } else if (isalpha(lbuffer[0])) {
    result->type = CMD;
    read_until_break(rdr, lbuffer + 1); 
  } else if (lbuffer[0] == '-' || isdigit(lbuffer[0])) {
    result->type = VAL;
    result->val.type = BLI32;
    read_until_break(rdr, lbuffer + 1);
    result->val.data.idata = atoi(lbuffer);
  } else {
    printf("unknown token type at %li", ftell(rdr));
    return false;
  }

  //printf("%i %s\n", result->type, lbuffer);
  return true;
}

void trim_next(FILE* rdr) {
  while (!feof(rdr) && isspace(peekc(rdr))) {
    fgetc(rdr);
  }
}

bool next_token(FILE* rdr, struct LexItem* item) {
  trim_next(rdr);
  return lex_next(rdr, item);
}

void add_end(struct program* out) {
  program_push(out, EXIT);
}

void push_i32(struct program* out, int32_t v) {
  uint8_t* start = program_expand(out, 2 + sizeof(int32_t));
  *start = PUSH_CONST;
  *(start + 1) = BLI32;
  *((int32_t*)(start + 2)) = v;
}

void pushr(struct program* out, int32_t v) {
  uint8_t* start = program_expand(out, 1 + sizeof(int32_t));
  *start = PUSH_RELATIVE_SP;
  *((int32_t*)(start + 1)) = v;
}

//FIXME: Does not remove resolved labels
/**
 * Given a list of labels to resolve (names and positions in a program of 32 bit labels)
 * Find named labels and rewrite the program to them
 */
void resolve_labels(struct program* out, struct labels* label_list, struct labels* unresolved) {
  struct Label* tl;
  struct Label* cl;

  //printf("Resolving labels\n");

  for (size_t i = 0; i < labels_size(unresolved); i++) {
    tl = &unresolved->data[i];
    for (size_t j = 0; j < labels_size(label_list); j++) {
      cl = &label_list->data[j];
      if (strcmp(tl->name, cl->name) == 0) {
        *((uint32_t*) (out->data + tl->pos)) = cl->pos;
        printf("Resolved a label\n");
        printf("%li\n", *((uint32_t*) (out->data + tl->pos)));	
      } else {
        //printf("Not resolved %s %s\n", tl->name, cl->name);
      }
    }
  }
  //printf("Done label resolution\n");
}

void build_jmp(enum BLOP type, FILE* rdr, struct program* out, struct labels* label_list, struct labels* unresolved) {

  struct LexItem item;
  struct Label nl;

  if (next_token(rdr, &item) && item.type == CMD) {
    printf("Build JNE %i\n", type);
    program_push(out, type);
    size_t ref_start = program_size(out);
    program_expand(out, 4);

    strcpy(nl.name, item.lbuffer);
    nl.pos = ref_start;
    labels_push(unresolved, nl);
    resolve_labels(out, label_list, unresolved);
  } else {
    printf("JMP expects label\n");
  }
}

void parse(FILE* rdr, struct program* out) {
  struct LexItem item;
  struct Label nl;
  struct labels label_list;
  struct labels unresolved;
  labels_init(&label_list);
  labels_init(&unresolved);
  while (!feof(rdr)) {
    
    if (!next_token(rdr, &item)) {
      break;
    }

    if (peekc(rdr) == ':') {
      //label
      strcpy(nl.name, item.lbuffer);
      nl.pos = program_size(out);
      labels_push(&label_list, nl);
      printf("Label %s at %zu\n", nl.name, nl.pos);
      fgetc(rdr);
      resolve_labels(out, &label_list, &unresolved);
    } else { //Command
      if (strcmp(item.lbuffer, "push") == 0) {
        if (next_token(rdr, &item) && item.type == VAL) {
          push_i32(out, item.val.data.idata);
        } else {
          printf("Lex error push not followed by value\n");
	  return;
        }
      } if (strcmp(item.lbuffer, "pushr") == 0) {
        if (next_token(rdr, &item) && item.type == VAL) {
          pushr(out, item.val.data.idata);
        } else {
          printf("Lex error push not followed by value\n");
	  return;
        }
      } else if (strcmp(item.lbuffer, "add") == 0) {
        program_push(out, ADD);
      } else if (strcmp(item.lbuffer, "sub") == 0) {
        program_push(out, SUB);
      } else if (strcmp(item.lbuffer, "mul") == 0) {
        program_push(out, MUL);
      } else if (strcmp(item.lbuffer, "div") == 0) {
        program_push(out, DIV);
      } else if (strcmp(item.lbuffer, "pop") == 0) {
	program_push(out, POP);
      }	else if (strcmp(item.lbuffer, "jmp") == 0) {
        build_jmp(JMP, rdr, out, &label_list, &unresolved);	
      } else if (strcmp(item.lbuffer, "je") == 0) {
        build_jmp(JE, rdr, out, &label_list, &unresolved);
      } else if (strcmp(item.lbuffer, "jne") == 0) {
        build_jmp(JNE, rdr, out, &label_list, &unresolved);
      }
    }
  }
}

int main(int argc, char** argv) {
  printf("Launching BLAS\n");
  struct program p1;
  program_init(&p1);
  FILE* fin = fopen(argv[1], "r");
  printf("Lex next line\n");
  parse(fin, &p1);
  add_end(&p1);

  FILE* fout = fopen(argv[2], "w");

  if (!fout) {
    printf("Could not open file\n");
  }

  if (fwrite(p1.data, program_size(&p1), 1, fout) != 1) {
    printf("Could not write file\n");
    return 1;
  }
  return 0;
}
