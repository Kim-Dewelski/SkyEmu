#include "arm_jit_backend_interp.h"
#include "arm_jit.h"

#include <stdint.h>
#include <assert.h>

#define MAX_HOST_BLOCK_SIZE 4096

void run_block(jitarm_backend_block_t* blk);

typedef struct {
  jitarm_ir_value_type_t values [JITARM_BLOCK_MAX_IR];
  bool should_stop;
} block_state_t;

typedef struct {
  jitarm_ir_index_t ir_index;
  const jitarm_ir_t* ir;
  const jitarm_block_t* block;
  block_state_t* block_state;
} handler_arg_t;

typedef void(*handler_fn_t)(handler_arg_t*);

typedef struct {
  handler_fn_t fn;
  handler_arg_t arg;
} handler_t;

struct jitarm_backend_block {
  block_state_t state;
  handler_t handlers[JITARM_BLOCK_MAX_IR];
  size_t len;
};

typedef struct {
  size_t idx;
  jitarm_backend_block_t* block;
} block_writer_t; 

void block_write(block_writer_t* writer, handler_fn_t handler, handler_arg_t arg) {
  assert(writer->idx < JITARM_BLOCK_MAX_IR);
  writer->block->handlers[writer->idx].fn = handler;
  writer->block->handlers[writer->idx].arg = arg;
}

// --------- IMPLEMENTATION ---------

void handler_binary(handler_arg_t* arg) {
  jitarm_ir_value_type_t lhs = arg->block_state->values[arg->ir->binary.lhs];
  jitarm_ir_value_type_t rhs = arg->block_state->values[arg->ir->binary.rhs];
  jitarm_ir_value_type_t dst;

  switch (arg->ir->binary.op) {
    case JITARM_IR_BINARY_ADD: dst = lhs + rhs; break;
    case JITARM_IR_BINARY_SUB: dst = lhs - rhs; break;
    case JITARM_IR_BINARY_MUL: dst = lhs * rhs; break;
    case JITARM_IR_BINARY_AND: dst = lhs & rhs; break;
    case JITARM_IR_BINARY_OR:  dst = lhs | rhs; break;
    case JITARM_IR_BINARY_XOR: dst = lhs ^ rhs; break;
  }

  arg->block_state->values[arg->ir_index] = dst;
}

void handler_unary(handler_arg_t* arg) {
  jitarm_ir_value_type_t src = arg->block_state->values[arg->ir->unary.src];
  jitarm_ir_value_type_t dst;

  switch (arg->ir->unary.op) {
    case JITARM_IR_UNARY_NOT: dst = ~src; break;
  }

  arg->block_state->values[arg->ir_index] = dst;
}

void handler_jump(handler_arg_t* arg) {
  run_block(arg->block->backend_block);
}

void handler_load_constant(handler_arg_t* arg) {}

void emit_ir(block_writer_t* writer, handler_arg_t arg) {
  switch (arg.ir->kind) {
    case JITARM_IR_KIND_LOAD_CONSTANT: block_write(writer,  handler_load_constant, arg); break;
    case JITARM_IR_KIND_JUMP: block_write(writer, handler_jump, arg); break;
    case JITARM_IR_KIND_BINARY: block_write(writer, handler_binary, arg); break;
    case JITARM_IR_KIND_UNARY: block_write(writer, handler_unary, arg); break;
  }
}

// ----------- INTERFACE ------------

struct {
  jitarm_backend_block_t host_blocks[JITARM_BLOCKS];
} ctx;

jitarm_backend_block_t* compile_block(size_t idx, const jitarm_block_t* blk) {
  jitarm_backend_block_t* host = &ctx.host_blocks[idx];

  block_writer_t writer;
  writer.block = host;
  writer.idx = 0;
  jitarm_ir_index_t ir_idx = blk->head;
  for(int i = 0; i < blk->length; ++i) {
    handler_arg_t arg;
    arg.block = blk;
    arg.ir_index = ir_idx;
    arg.ir = &blk->ir[ir_idx];
    arg.block_state = &host->state;
    emit_ir(&writer, arg);
    ir_idx = blk->ir[ir_idx].next;
  }
  return host;
}

void run_block(jitarm_backend_block_t* blk) {
  for (int i = 0; i < blk->len; ++i) {
    blk->handlers[i].fn(&blk->handlers[i].arg);
  }
}

jitarm_backend_t backend = { 
  .compile_block = compile_block,
  .run_block = run_block,
};

const jitarm_backend_t* jitarm_backend_interpreter() {
  return &backend;
}
