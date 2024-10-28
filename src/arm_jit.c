#include "arm_jit.h"

#include <stdbool.h>
#include <assert.h>
#include <stdint.h>

// how many blocks we can compile concurrently
#define JITARM_BLOCKS 256
#define CACHE_SIZE 256

typedef struct {
  jitarm_backend_block_t* backend_block;
  uint32_t adr;
  bool valid;
} cached_t;

struct {
  jitarm_block_t blocks[JITARM_BLOCKS];
  unsigned block_alloc_index;
  const jitarm_backend_t* backend;
  cached_t cached[CACHE_SIZE];
} ctx;

// ------------------ UTILITY ------------------

jitarm_ir_index_t emit_ir(jitarm_block_index_t block, jitarm_ir_t ir) {
  jitarm_block_t* ptr = &ctx.blocks[block];
  if (ptr->length < JITARM_BLOCK_MAX_IR) {
    ptr->ir[ptr->length] = ir;
    if (ptr->length > 0) {
      ptr->ir[ptr->length - 1].next = ptr->length;
    } else {
      ptr->head = ptr->length;
    }
    return ptr->length++;
  } else {
    // if this assert triggers, we need to increase the amount of IR a block can handle to something larger.
    assert(false && "IR block size is insufficient, this is a bug!");
  }
}

void compile_block(jitarm_frontend_t* frontend, uint32_t adr) {
    
}

// ----------------- INTERFACE -----------------

void jitarm_set_backend(const jitarm_backend_t* backend) {
  ctx.backend = backend;
}

void jitarm_run(jitarm_frontend_t* frontend, uint32_t adr) {
  for(int i = 0; i < CACHE_SIZE; ++i) {
    if (ctx.cached[i].valid && ctx.cached[i].adr == adr) {
      ctx.backend->run_block(ctx.cached[i].backend_block);
      return;
    }
  }

  // if the block was not found, we will have to compile the block.
  compile_block(frontend, adr);
  jitarm_run(frontend, adr);
}

jitarm_block_index_t jitarm_block_alloc() {
  unsigned index = ctx.block_alloc_index++ & (JITARM_BLOCKS - 1);
  return index;
}

void jitarm_block_compile(jitarm_block_index_t blk) {
  jitarm_backend_block_t* gen = ctx.backend->compile_block(blk, &ctx.blocks[blk]);
  ctx.blocks[blk].backend_block = gen;
}

void jitarm_block_passes(jitarm_block_index_t, const jitarm_passes_t* passes) {}

jitarm_ir_index_t jitarm_ir_emit_load_constant(jitarm_block_index_t blk, jitarm_ir_value_type_t value) {
  jitarm_ir_t ir;
  ir.kind = JITARM_IR_KIND_LOAD_CONSTANT;
  ir.next = 0;
  ir.constant.constant = value;
  return emit_ir(blk, ir);
}

jitarm_ir_index_t jitarm_ir_emit_unary(jitarm_block_index_t blk, jitarm_ir_unary_t op, jitarm_ir_index_t src) {
  jitarm_ir_t ir;
  ir.kind = JITARM_IR_KIND_UNARY;
  ir.next = JITARM_INVALID_IDX;
  ir.unary.op = op;
  ir.unary.src = src;
  return emit_ir(blk, ir);
}

jitarm_ir_index_t jitarm_ir_emit_binary(jitarm_block_index_t blk, jitarm_ir_binary_t op, jitarm_ir_index_t lhs, jitarm_ir_index_t rhs) {
  jitarm_ir_t ir;
  ir.kind = JITARM_IR_KIND_BINARY;
  ir.next = JITARM_INVALID_IDX;
  ir.binary.op = op;
  ir.binary.lhs = lhs;
  ir.binary.rhs = rhs;
  return emit_ir(blk, ir);
}
