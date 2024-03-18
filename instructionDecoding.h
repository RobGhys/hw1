//
// Created by rob on 18/03/24.
//

#ifndef HW1_INSTRUCTIONDECODING_H
#define HW1_INSTRUCTIONDECODING_H

#include <iostream>
#include <bitset>


struct TwoBytes {
    std::bitset<8> firstByte;
    std::bitset<8> secondByte;
};

enum OperationMod {
    RegisterMode,
    MemoryModeNoDisplacement,
    MemoryMode8Bit,
    MemoryMode16Bit
};

enum OperationName {
    MovRegisterToRegister,
    MovImmediateToRegister,
    JumpInstruction,
    AddRegisterToRegister,
    XImmediateToRegisterOrMemory,
    AddImmediateToAccumulator,
    SubRegMemoryAndRegToEither,
    SubImmediateFromAccumulator,
    CmpRegisterMemoryAndRegister,
    CmpImmediateWithAccumulator,
    NotFound
};

struct X8086Instruction {
    OperationName operation{}; // mov
    /*
     * Determination bit
     * 0 -> reg register is the source
     * 1 -> reg register is the destination
     */
    int dBit{};
    /*
     * Wide instruction bit
     * 0 -> mov will copy 8 bits
     * 1 -> mov will copy 16 bits
     */
    int wBit{};
    int sBit{};
    std::string sourceReg;
    std::string destReg;
    OperationMod operationMod{};
};

X8086Instruction &decodeJumpInstruction(X8086Instruction &instruction, const TwoBytes &sixteenBits);
void decodeImmediateInstruction(const TwoBytes &sixteenBits, std::ifstream &inputFile, X8086Instruction &instruction);
int getModAndDecodeExtraBytes(const TwoBytes &inputBits, X8086Instruction &instruction);
void outputImmediateToReg(const TwoBytes &sixteenBits, std::ifstream &inputFile, X8086Instruction &instruction, const std::string &instructionType);
void outputRegToReg(const TwoBytes &sixteenBits, std::ifstream &inputFile, X8086Instruction &instruction, const std::string& instructionType);
bool decodeImmediateToRegInstruction(const TwoBytes &inputBits, X8086Instruction &instruction);
void decodeRegToRegMovInstruction(const TwoBytes &inputBits, X8086Instruction &instruction,
                                  const std::string& byteDisplacement);
void decodeImmediateToAcc(const TwoBytes &sixteenBits, std::ifstream &inputFile, X8086Instruction &instruction, const std::string &operationType);
bool checkIfJump(const TwoBytes &inputBits);
bool checkIfImmediateMov(const TwoBytes &inputBits);

#endif //HW1_INSTRUCTIONDECODING_H
