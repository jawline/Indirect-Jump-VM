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

int peekc(FILE* rdr) {
  int c = fgetc(rdr);
  ungetc(c, rdr);
  return c;
}

void read_until_break(FILE* rdr, char* lbuffer) {
  char nextc = peekc(rdr);
  while (nextc != EOF && nextc != ',' && nextc != ' ' && nextc != '\n') {
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
  } else if (isdigit(lbuffer[0])) {
    result->type = VAL;
    result->val.type = BLI32;
    read_until_break(rdr, lbuffer + 1);
    result->val.data.idata = atoi(lbuffer);
  } else {
    printf("unknown token type at %li", ftell(rdr));
    return false;
  }

  printf("%i %s\n", result->type, lbuffer);
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

void lex_next_line(FILE* rdr, struct program* out) {
  struct LexItem item; 
  while (!feof(rdr)) {
    if (!next_token(rdr, &item)) {
      break;
    }
    if (strcmp(item.lbuffer, "push") == 0) {
      if (next_token(rdr, &item) && item.type == VAL) {
	push_i32(out, item.val.data.idata);
	printf("push %i\n", item.val.data.idata);
      } else {
        printf("Lex error push not followed by value\n");
      }
    } else if (strcmp(item.lbuffer, "add") == 0) {
      program_push(out, ADD);
    } else if (strcmp(item.lbuffer, "sub") == 0) {
      program_push(out, SUB);
    } else if (strcmp(item.lbuffer, "mul") == 0) {
      program_push(out, MUL);
    } else if (strcmp(item.lbuffer, "div") == 0) {
      program_push(out, DIV);
    }
  }
}

int main(int argc, char** argv) {
  printf("Launching BLAS\n");
  struct program p1;
  program_init(&p1);
  FILE* fin = fopen(argv[1], "r");
  printf("Lex next line\n");
  lex_next_line(fin, &p1);
  FILE* fout = fopen(argv[2], "w");
  fwrite(p1.data, program_size(&p1), 1, fout);
}
