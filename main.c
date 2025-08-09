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

int32_t sign_extend(uint32_t val, int bits) {
    int32_t m = 1u << (bits - 1);
    return (val ^ m) - m;
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
        i->imm    = (inst >> 20);
    }
    // Handle S-type (opcode: 0x23)
    else if (opcode == 0x23) {
        SType *s = (SType *)decoded_inst;
        s->opcode = opcode;
        s->funct3 = (inst >> 12) & 0x7;
        s->rs1    = (inst >> 15) & 0x1f;
        s->rs2    = (inst >> 20) & 0x1f;
        s->imm    = ((inst >> 7) & 0x1f) 
                  | (((inst >> 25) & 0x7f) << 5);
    }
    // Handle B-type (opcode: 0x63)
    else if (opcode == 0x63) {
        BType *b = (BType *)decoded_inst;
        b->opcode = opcode;
        b->funct3 = (inst >> 12) & 0x7;
        b->rs1    = (inst >> 15) & 0x1f;
        b->rs2    = (inst >> 20) & 0x1f;
        b->imm    = ((inst >> 8) & 0xf) 
                  | (((inst >> 25) & 0x3f) << 4)
                  | (((inst >> 7) & 0x1) << 10)
                  | ((inst >> 31) << 11);
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
        j->imm    = ((inst >> 21) & 0x3ff) 
                  | ((inst >> 20) & 0x1) << 10 
                  | ((inst >> 12) & 0xff) << 11
                  | ((inst >> 31) << 19);
    }
    else {
        // Unrecognized opcode, handle error
        printf("[-] Unknown opcode: 0x%02X\n", opcode);
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
        if (r->funct3 == 0x0) { // ADD/SUB
          if (r->funct7 == 0x0) {
            regs[r->rd] = regs[r->rs1] + regs[r->rs2];
            puts("[*] Executed ADD");
          }
          else if (r->funct7 == 0x20) {
            regs[r->rd] = regs[r->rs1] - regs[r->rs2];
            puts("[*] Executed SUB");
          }
        }
        else if (r->funct3 == 0x7) { // AND
          regs[r->rd] = regs[r->rs1] & regs[r->rs2];
          puts("[*] Executed AND");
        }
        else if (r->funct3 == 0x6) { // OR
          regs[r->rd] = regs[r->rs1] | regs[r->rs2];
          puts("[*] Executed OR");
        }
        else if (r->funct3 == 0x4) { // XOR
          regs[r->rd] = regs[r->rs1] ^ regs[r->rs2];
          puts("[*] Executed XOR");
        }
        else {
          puts("[-] Unknown Instruction");
        }
        break;
      }
    case 0x13: // I-type
      {
        IType *i = (IType*) decoded_inst;
        // Handle I-type instructions (e.g. ADDI)
        if (i->funct3 == 0x0) { // ADDI
          regs[i->rd] = (int32_t)regs[i->rs1] + sign_extend(i->imm, 12);
          puts("[*] Executed ADDI");
        } 
        else if (i->funct3 == 0x2) { // SLTI
          regs[i->rd] = regs[i->rs1] < sign_extend(i->imm, 12) ? 1 : 0;
          puts("[*] Executed SLTI");
        }
        else if (i->funct3 == 0x3) { // SLTIU
          regs[i->rd] = regs[i->rs1] < i->imm ? 1 : 0;
          puts("[*] Executed SLTIU");
        }
        break;
      }
    case 0x23: // S-type
      {
        SType *s = (SType*) decoded_inst;
        // Handle S-type instructions (e.g. SW)
        store_word(regs[s->rs1] + sign_extend(s->imm, 12), regs[s->rs2]);
      }
    default:
      printf("[-] Unknown opcode: %x\n", opcode);
      break;
  }

  pc += 4; // move to the next instruction
  regs[REG_ZERO] = 0;

}

void* allocate_instruction(uint32_t opcode) {
    switch (opcode) {
        case 0x33: // R-type
            return malloc(sizeof(RType));
        case 0x13: // I-type (e.g. ADDI)
        case 0x03: // I-type (e.g. LOADs)
            return malloc(sizeof(IType));
        case 0x23: // S-type
            return malloc(sizeof(SType));
        case 0x63: // B-type
            return malloc(sizeof(BType));
        case 0x37: // U-type (LUI)
        case 0x17: // U-type (AUIPC)
            return malloc(sizeof(UType));
        case 0x6f: // J-type (JAL)
            return malloc(sizeof(JType));
        default:
            printf("[-] Unknown opcode: 0x%02X\n", opcode);
            return NULL;
    }
}

int main(int argc, char *argv[])
{
  puts("RISC-V Enulator\n");

  // Instructions to execute:
  uint32_t program[] = {
      0b00000000101000000000000010010011,  // addi x1, x0, 10
      0b00000000111100000000000100010011,  // addi x2, x0, 15
      0b00000000001000001000000110110011,  // add x3, x2, x1
      0b01000000001000001000001000110011   // sub x4, x2, x1
  };

  // Load the instructions into memory
  for (int i = 0; i < 4; i++) {
      memory[i * 4]     = program[i] & 0xFF;
      memory[i * 4 + 1] = (program[i] >> 8) & 0xFF;
      memory[i * 4 + 2] = (program[i] >> 16) & 0xFF;
      memory[i * 4 + 3] = (program[i] >> 24) & 0xFF;
  }

  while (pc < MEM_SIZE) {
    uint32_t inst = load_word(pc);
    if(inst == 0) break; // halt condition
    
    uint32_t opcode = inst & 0x7f;

    void *decoded_inst = allocate_instruction(opcode);
    
    decode_instruction(inst, decoded_inst);
    execute(decoded_inst);

    free(decoded_inst);
  }

  printf("x1 = %d\n", regs[1]);
  printf("x2 = %d\n", regs[2]);
  printf("x3 = %d\n", regs[3]);
  printf("x4 = %d\n", regs[4]);

  return 0;
}
