//
// Created by rob on 18/03/24.
//

#ifndef HW1_INSTRUCTIONDECODING_H
#define HW1_INSTRUCTIONDECODING_H

#include <iostream>
#include <bitset>
#include <unordered_map>


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

std::string decodeJumpInstruction(X8086Instruction &instruction, const TwoBytes &sixteenBits);
std::string decodeImmediateInstruction(const TwoBytes &sixteenBits, std::ifstream &inputFile, X8086Instruction &instruction);
int getModAndDecodeExtraBytes(const TwoBytes &inputBits, X8086Instruction &instruction);
std::string outputImmediateToReg(const TwoBytes &sixteenBits, std::ifstream &inputFile, X8086Instruction &instruction, const std::string &instructionType, std::unordered_map<std::string, int> &registerValueMap);
std::string outputRegToReg(const TwoBytes &sixteenBits, std::ifstream &inputFile, X8086Instruction &instruction, const std::string& instructionType, std::unordered_map<std::string, int> &registerValueMap);
bool decodeImmediateToRegInstruction(const TwoBytes &inputBits, X8086Instruction &instruction);
void decodeRegToRegMovInstruction(const TwoBytes &inputBits, X8086Instruction &instruction,
                                  const std::string& byteDisplacement);
std::string decodeImmediateToAcc(const TwoBytes &sixteenBits, std::ifstream &inputFile, X8086Instruction &instruction, const std::string &operationType, std::unordered_map<std::string, int> &registerValueMap);
bool checkIfJump(const TwoBytes &inputBits);
bool checkIfImmediateMov(const TwoBytes &inputBits);

#endif //HW1_INSTRUCTIONDECODING_H
