//
// Created by rob on 18/03/24.
//

#ifndef HW1_INSTRUCTIONDECODING_H
#define HW1_INSTRUCTIONDECODING_H

#include <iostream>
#include <bitset>
#include <unordered_map>
#include <vector>

struct InstructionFlags {
    bool signFlag = false;
    bool zeroFlag = false;
};

struct ProgramOutput {
    std::vector<std::string> instructionPrinter;
    std::unordered_map<std::string, int> registerValueMap;
    InstructionFlags flags;
};

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
void decodeImmediateInstruction(const TwoBytes &sixteenBits, std::ifstream &inputFile, X8086Instruction &instruction, ProgramOutput &programOutput);
int getModAndDecodeExtraBytes(const TwoBytes &inputBits, X8086Instruction &instruction);
void outputImmediateToReg(const TwoBytes &sixteenBits, std::ifstream &inputFile, X8086Instruction &instruction, const std::string &instructionType, ProgramOutput &programOutput);
void outputRegToReg(const TwoBytes &sixteenBits, std::ifstream &inputFile, X8086Instruction &instruction, const std::string& instructionType, ProgramOutput &programOutput);
bool decodeImmediateToRegInstruction(const TwoBytes &inputBits, X8086Instruction &instruction);
void decodeRegToRegMovInstruction(const TwoBytes &inputBits, X8086Instruction &instruction, const std::string& byteDisplacement);
void decodeImmediateToAcc(const TwoBytes &sixteenBits, std::ifstream &inputFile, X8086Instruction &instruction, const std::string &operationType, ProgramOutput &programOutput);
bool checkIfJump(const TwoBytes &inputBits);
bool checkIfImmediateMov(const TwoBytes &inputBits);

#endif //HW1_INSTRUCTIONDECODING_H
