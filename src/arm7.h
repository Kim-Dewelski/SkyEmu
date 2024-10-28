#ifndef ARM7_H
#define ARM7_H 1

#include "arm_jit.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

////////////////
// Data Types //
////////////////

#define LR 14
#define PC 15
#define CPSR 16
#define SPSR 17 

#define R13_fiq 22 
#define R13_irq 24 
#define R13_svc 26 
#define R13_abt 28 
#define R13_und 30 

#define R14_fiq 23 
#define R14_irq 25 
#define R14_svc 27 
#define R14_abt 29 
#define R14_und 31 

#define SPSR_fiq 32 
#define SPSR_irq 33 
#define SPSR_svc 34 
#define SPSR_abt 35 
#define SPSR_und 36 

#define UNINTIALIZED_PREFETCH_PC -3
// Memory IO functions for the emulated CPU (these must be defined by the user)
typedef uint32_t (*arm_read32_fn_t)(void* user_data, uint32_t address);
typedef uint32_t (*arm_read16_fn_t)(void* user_data, uint32_t address);
typedef uint32_t (*arm_read32_seq_fn_t)(void* user_data, uint32_t address,bool is_sequential);
typedef uint32_t (*arm_read16_seq_fn_t)(void* user_data, uint32_t address,bool is_sequential);
typedef uint8_t (*arm_read8_fn_t)(void* user_data, uint32_t address);
typedef void (*arm_write32_fn_t)(void* user_data, uint32_t address, uint32_t data);
typedef void (*arm_write16_fn_t)(void* user_data, uint32_t address, uint16_t data);
typedef void (*arm_write8_fn_t)(void* user_data, uint32_t address, uint8_t data);
typedef uint32_t (*arm_coproc_read_fn_t)(void* user_data, int coproc,int opcode,int Cn, int Cm,int Cp);
typedef void (*arm_coproc_write_fn_t)(void* user_data, int coproc,int opcode,int Cn, int Cm,int Cp, uint32_t data);
typedef void (*arm_trigger_breakpoint_fn_t)(void* user_data);


#define ARM_DEBUG_BRANCH_RING_SIZE 32
#define ARM_DEBUG_SWI_RING_SIZE 32
typedef struct arm7 {
  // Registers
  /*
  0-15: R0-R15
  16: CPSR
  17-23: R8_fiq-R14_fiq
  24-25: R13_irq-R14_irq
  26-27: R13_svc-R14_svc
  28-29: R13_abt-R14_abt
  30-31: R13_und-R14_und
  32: SPSR_fiq
  33: SPSR_irq
  34: SPSR_svc
  35: SPSR_abt
  36: SPSR_und
  */

  uint32_t debug_branch_ring[ARM_DEBUG_BRANCH_RING_SIZE];
  uint32_t debug_branch_ring_offset;
  uint32_t debug_swi_ring[ARM_DEBUG_SWI_RING_SIZE];
  uint32_t debug_swi_ring_times[ARM_DEBUG_SWI_RING_SIZE];
  uint32_t debug_swi_ring_offset;
  uint32_t prefetch_pc;
  uint32_t step_instructions;//Instructions to step before triggering a breakpoint
  uint32_t prefetch_opcode[5]; 
  uint32_t i_cycles;//Executed i-cycles minus 1
  bool next_fetch_sequential;
  uint32_t registers[37];
  uint64_t executed_instructions;
  bool print_instructions;
  void* user_data;
  FILE* log_cmp_file;
  arm_read32_fn_t     read32;
  arm_read16_fn_t     read16;
  arm_read32_seq_fn_t read32_seq;
  arm_read16_seq_fn_t read16_seq;
  arm_read8_fn_t      read8;
  arm_write32_fn_t    write32;
  arm_write16_fn_t    write16;
  arm_write8_fn_t     write8;
  arm_coproc_read_fn_t coprocessor_read;
  arm_coproc_write_fn_t coprocessor_write;
  arm_trigger_breakpoint_fn_t trigger_breakpoint; 
  bool wait_for_interrupt; 
  uint32_t irq_table_address; 
  uint32_t phased_opcode; 
  uint32_t phased_op_id; 
  uint32_t phase; 
  struct{
    uint32_t addr;
    uint32_t r15_off;
    uint32_t last_bank;
    uint32_t base_addr;
    uint32_t cycle;
    uint32_t num_regs;
  }block;
} arm7_t;     

typedef void (*arm7_handler_t)(arm7_t *cpu, uint32_t opcode);
typedef struct{
	arm7_handler_t handler;
	arm7_handler_t jit_handler;
	char name[12];
	char bitfield[33];
}arm7_instruction_t;

#define ARM_PHASED_NONE      0 
#define ARM_PHASED_FILL_PIPE 1
#define ARM_PHASED_BLOCK_TRANSFER 2

////////////////////////
// User API Functions //
////////////////////////

// This function initializes the internal state needed for the arm7 core emulation
arm7_t arm7_init(void* user_data);
// This function executed an interpreter instruction for arm7
void arm7_exec_instruction(arm7_t* cpu);
// This function compiles 
jitarm_block_index_t arm7_jit_compile(arm7_t* cpu);

// Write the dissassembled opcode from mem_address into the out_disasm string up to out_size characters
static void arm7_get_disasm(arm7_t * cpu, uint32_t mem_address, char* out_disasm, size_t out_size);
// Used to send an interrupt to the emulated CPU. The n'th set bit triggers the n'th interrupt
void arm7_process_interrupts(arm7_t* cpu);

#endif
