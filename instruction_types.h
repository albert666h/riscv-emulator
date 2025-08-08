#ifndef INSTRUCTION_TYPES_H
#define INSTRUCTION_TYPES_H

#include <stdint.h>

// R-type: register-register arithmetic
typedef struct {
    uint32_t opcode;
    uint32_t rd;
    uint32_t funct3;
    uint32_t rs1;
    uint32_t rs2;
    uint32_t funct7;
} RType;

// I-type: immediate arithmetic, loads
typedef struct {
    uint32_t opcode;
    uint32_t rd;
    uint32_t funct3;
    uint32_t rs1;
    int32_t  imm; // sign-extended
} IType;

// S-type: stores
typedef struct {
    uint32_t opcode;
    uint32_t funct3;
    uint32_t rs1;
    uint32_t rs2;
    int32_t  imm; // split + sign-extended
} SType;

// B-type: conditional branches
typedef struct {
    uint32_t opcode;
    uint32_t funct3;
    uint32_t rs1;
    uint32_t rs2;
    int32_t  imm; // sign-extended branch offset
} BType;

// U-type: upper immediate instructions
typedef struct {
    uint32_t opcode;
    uint32_t rd;
    int32_t  imm; // upper 20 bits shifted left by 12
} UType;

// J-type: jump (JAL)
typedef struct {
    uint32_t opcode;
    uint32_t rd;
    int32_t  imm; // sign-extended jump offset
} JType;

#endif // INSTRUCTION_TYPES_H

