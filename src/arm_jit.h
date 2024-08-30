#include <stdint.h>
#include <stddef.h>

// forward declarations.
struct jitarm_block;

typedef uint64_t jitarm_ir_value_type_t;

typedef struct jitarm_ctx jitarm_ctx_t;

typedef struct {
  uint16_t index;
} jitarm_ir_value_t;

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
  JITARM_IR_LOAD_CONSTANT,
  JITARM_IR_KIND_UNARY,
  JITARM_IR_KIND_BINARY,
  JITARM_IR_JUMP,
} jitarm_ir_kind_t;

typedef struct {
  jitarm_ir_kind_t kind;
  union {
    struct {
      jitarm_ir_value_t dst;
      jitarm_ir_value_type_t constant;
    } constant;
    struct {
      jitarm_ir_unary_t op;
      jitarm_ir_value_t dst;
      jitarm_ir_value_t src;
    } unary;
    struct {
      jitarm_ir_binary_t op;
      jitarm_ir_value_t dst;
      jitarm_ir_value_t lhs;
      jitarm_ir_value_t rhs;
    } binary;
    struct {
      struct jitarm_block* block;
    } jump;
  };
} jitarm_ir_t;

typedef struct jitarm_block {
  size_t length;
  jitarm_ir_t* ir;
} jitarm_block_t;

jitarm_ctx_t* jitarm_create_ctx();
void jitarm_destroy_ctx(jitarm_ctx_t*);

void jitarm_flush(jitarm_ctx_t*);

jitarm_block_t* jitarm_ir_create_block(jitarm_ctx_t*);
void jitarm_ir_destroy_block(jitarm_ctx_t*, jitarm_block_t*);

jitarm_ir_value_t jitarm_ir_emit_load_constant(jitarm_block_t*, jitarm_ir_value_t, jitarm_ir_value_type_t);
jitarm_ir_value_t jitarm_ir_emit_unary(jitarm_block_t*, jitarm_ir_value_t src);
jitarm_ir_value_t jitarm_ir_emit_binary(jitarm_block_t*, jitarm_ir_value_t lhs, jitarm_ir_value_t rhs);
void jitarm_ir_emit_jump(jitarm_block_t*, jitarm_block_t* block);
