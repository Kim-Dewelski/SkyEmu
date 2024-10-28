#ifndef ARM_JIT_H
#define ARM_JIT_H

// NOTE: must be a power of two.
#define JITARM_BLOCK_MAX_IR 512
#define JITARM_INVALID_IDX (~(jitarm_ir_index_t)0)

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// type alias
typedef uint64_t jitarm_ir_value_type_t;
typedef size_t jitarm_ir_index_t;
typedef size_t jitarm_block_index_t;

// forward declarations.
typedef struct jitarm_block jitarm_block_t;

// a struct filled with frontend callbacks
typedef struct {} jitarm_frontend_t;

typedef enum {
  JITARM_IR_UNARY_NOT,
} jitarm_ir_unary_t;

typedef enum {
  JITARM_IR_BINARY_ADD,
  JITARM_IR_BINARY_SUB,
  JITARM_IR_BINARY_MUL,
  JITARM_IR_BINARY_AND,
  JITARM_IR_BINARY_OR,
  JITARM_IR_BINARY_XOR,
} jitarm_ir_binary_t;

typedef enum {
  JITARM_IR_KIND_LOAD_CONSTANT,
  JITARM_IR_KIND_LOAD_MEMORY,
  JITARM_IR_KIND_STORE_MEMORY,
  JITARM_IR_KIND_LOAD_REGISTER,
  JITARM_IR_KIND_STORE_REGISTER,
  JITARM_IR_KIND_UNARY,
  JITARM_IR_KIND_BINARY,
} jitarm_ir_kind_t;

typedef enum {
  JITARM_HOST_REGISTER_R0,
  JITARM_HOST_REGISTER_R1,
  JITARM_HOST_REGISTER_R2,
  JITARM_HOST_REGISTER_R3,
  JITARM_HOST_REGISTER_R4,
  JITARM_HOST_REGISTER_R5,
  JITARM_HOST_REGISTER_R6,
  JITARM_HOST_REGISTER_R7,
  JITARM_HOST_REGISTER_R8,
  JITARM_HOST_REGISTER_R9,
  JITARM_HOST_REGISTER_R10,
  JITARM_HOST_REGISTER_R11,
  JITARM_HOST_REGISTER_R12,
  JITARM_HOST_REGISTER_R13,
  JITARM_HOST_REGISTER_R14,
  JITARM_HOST_REGISTER_R15,
} jitarm_host_register_t;

typedef struct {
  jitarm_ir_kind_t kind;
  jitarm_ir_index_t next;
  union {
    struct {
      jitarm_ir_value_type_t constant;
    } constant;
    struct {
      jitarm_ir_index_t adr;
    } memory_load;
    struct {
      jitarm_ir_index_t adr;
      jitarm_ir_index_t src;
    } memory_store;
    struct {
      jitarm_host_register_t src;
    } register_load;
    struct {
      jitarm_host_register_t dst;
      jitarm_ir_index_t src;
    } register_store;
    struct {
      jitarm_ir_unary_t op;
      jitarm_ir_index_t src;
    } unary;
    struct {
      jitarm_ir_binary_t op;
      jitarm_ir_index_t lhs;
      jitarm_ir_index_t rhs;
    } binary;
  };
} jitarm_ir_t;

typedef struct jitarm_passes {
  bool opt_constant_propogation;
} jitarm_passes_t;

typedef struct jitarm_backend_block jitarm_backend_block_t; 

typedef struct jitarm_backend {
  jitarm_backend_block_t* (*compile_block)(size_t index, const jitarm_block_t*);
  void (*run_block)(jitarm_backend_block_t* blk);
} jitarm_backend_t;

typedef enum {
  JITARM_IR_BLOCK_SUCCESSOR_KIND_CALL,
  JITARM_IR_BLOCK_SUCCESSOR_KIND_JMP,
} jitarm_ir_block_successor_kind_t;

typedef struct {
  jitarm_ir_block_successor_kind_t kind;
  jitarm_ir_index_t cond;
  jitarm_block_index_t case0;
  jitarm_block_index_t case1;
} jitarm_block_successor_t;

struct jitarm_block {
  size_t length;
  jitarm_ir_t ir[JITARM_BLOCK_MAX_IR];
  jitarm_backend_block_t* backend_block;
  jitarm_ir_index_t head;
  jitarm_block_successor_t successor;
};

void jitarm_set_backend(const jitarm_backend_t* backend);
void jitarm_run(jitarm_frontend_t* frontend, uint32_t adr);

jitarm_block_index_t jitarm_block_alloc();
void jitarm_block_compile(jitarm_block_index_t);
void jitarm_block_passes(jitarm_block_index_t, const jitarm_passes_t*);

jitarm_ir_index_t jitarm_ir_emit_load_constant(jitarm_block_index_t, jitarm_ir_value_type_t);
jitarm_ir_index_t jitarm_ir_emit_unary(jitarm_block_index_t, jitarm_ir_unary_t op, jitarm_ir_index_t src);
jitarm_ir_index_t jitarm_ir_emit_binary(jitarm_block_index_t, jitarm_ir_binary_t op, jitarm_ir_index_t lhs, jitarm_ir_index_t rhs);

#endif
