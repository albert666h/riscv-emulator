#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "instruction_types.h"

#define MEM_SIZE (1024 * 1024) // 1 MB

uint32_t pc = 0;
uint32_t regs[32];
uint8_t memory[MEM_SIZE]; // byte-addressable

#define REG_ZERO 0

uint32_t load_word(uint32_t addr)
{
  return memory[addr] |
    (memory[addr + 1] << 8) |
    (memory[addr + 2] << 16) |
    (memory[addr + 3] << 24) ;
}

void store_word(uint32_t addr, uint32_t val)
{
  memory[addr]   = val         & 0xff;
  memory[addr+1] = (val >> 8)  & 0xff;
  memory[addr+2] = (val >> 16) & 0xff;
  memory[addr+3] = (val >> 24) & 0xff;
}

uint32_t fetch()
{
  return load_word(pc);
}

void decode_instruction(uint32_t inst, void *decoded_inst)
{
    uint32_t opcode = inst & 0x7f;  // The last 7 bits contain the opcode

    // Handle R-type (opcode: 0x33)
    if (opcode == 0x33) {
        RType *r = (RType *)decoded_inst;
        r->opcode  = opcode;
        r->rd      = (inst >> 7) & 0x1f;
        r->funct3  = (inst >> 12) & 0x7;
        r->rs1     = (inst >> 15) & 0x1f;
        r->rs2     = (inst >> 20) & 0x1f;
        r->funct7  = (inst >> 25) & 0x7f;
    }
    // Handle I-type (opcode: 0x13, 0x03)
    else if (opcode == 0x13 || opcode == 0x03) {
        IType *i = (IType *)decoded_inst;
        i->opcode = opcode;
        i->rd     = (inst >> 7) & 0x1f;
        i->funct3 = (inst >> 12) & 0x7;
        i->rs1    = (inst >> 15) & 0x1f;
        i->imm    = (int32_t)(inst >> 20); // sign-extended
    }
    // Handle S-type (opcode: 0x23)
    else if (opcode == 0x23) {
        SType *s = (SType *)decoded_inst;
        s->opcode = opcode;
        s->funct3 = (inst >> 12) & 0x7;
        s->rs1    = (inst >> 15) & 0x1f;
        s->rs2    = (inst >> 20) & 0x1f;
        s->imm    = ((inst >> 7) & 0x1f) | ((inst >> 25) & 0x7f); // split immediate
    }
    // Handle B-type (opcode: 0x63)
    else if (opcode == 0x63) {
        BType *b = (BType *)decoded_inst;
        b->opcode = opcode;
        b->funct3 = (inst >> 12) & 0x7;
        b->rs1    = (inst >> 15) & 0x1f;
        b->rs2    = (inst >> 20) & 0x1f;
        b->imm    = ((inst >> 7) & 0x1f) | ((inst >> 25) & 0x3f); // split and sign-extend
    }
    // Handle U-type (opcode: 0x37, 0x17)
    else if (opcode == 0x37 || opcode == 0x17) {
        UType *u = (UType *)decoded_inst;
        u->opcode = opcode;
        u->rd     = (inst >> 7) & 0x1f;
        u->imm    = (inst >> 12) & 0xfffff; // Upper 20 bits
    }
    // Handle J-type (opcode: 0x6f)
    else if (opcode == 0x6f) {
        JType *j = (JType *)decoded_inst;
        j->opcode = opcode;
        j->rd     = (inst >> 7) & 0x1f;
        j->imm    = ((inst >> 21) & 0x3ff) | ((inst >> 20) & 0x1) << 10 | ((inst >> 12) & 0xff) << 11; // J-type immediate
    }
    else {
        // Unrecognized opcode, handle error
        printf("Unknown opcode: 0x%02X\n", opcode);
    }
}

void execute(void *decoded_inst)
{
  uint32_t opcode = * ((uint32_t*)decoded_inst) & 0x7f; // get opcode

  switch(opcode) {
    case 0x33: // R-type
      {
        RType *r = (RType*) decoded_inst;
        // Handle R-type instruction (e.g. ADD, SUB, etc)
        if (r->funct3 == 0x0) { // ADD
          regs[r->rd] = regs[r->rs1] + regs[r->rs2];
        }
        break;
      }
    case 0x13: // I-type
      {
        IType *i = (IType*) decoded_inst;
        // Handle I-type instructions (e.g. ADDI)
        if (i->funct3 == 0x0) { // ADDI
          regs[i->rd] = regs[i->rs1] + i->imm;
        }
        break;
      }
    case 0x23: // S-type
      {
        SType *s = (SType*) decoded_inst;
        // Handle S-type instructions (e.g. SW)
        store_word(regs[s->rs1] + s->imm, regs[s->rs2]);
      }
    default:
      printf("Unknown opcode: %x\n", opcode);
      break;
  }

  pc += 4; // move to the next instruction

}

int main(int argc, char *argv[])
{
  puts("RISC-V Enulator\n");

  // Example program (addi x1, x0, 10)
  memory[0] = 0x93; memory[1] = 0x00; memory[2] = 0xa0; memory[3] = 0x00; // ADDI

  while (pc < MEM_SIZE) {
    uint32_t inst = load_word(pc);
    if(inst == 0) break; // halt condition

    void *decoded_inst = malloc(sizeof(JType)); // Quick fix, biggest struct
    
    decode_instruction(inst, decoded_inst);
    execute(decoded_inst);

    free(decoded_inst);
  }

  printf("x1 = %d\n", regs[1]);

  return 0;
}
