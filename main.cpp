#include <iostream>
#include <fstream>
#include <string>
#include <bitset>

#include "instructionDecoding.h"
#include "byteReader.h"

OperationName getOperation(const TwoBytes &inputBits) {
    // MOV
    if (checkSixBitsInRegister(inputBits, std::bitset<6>(std::string("100010"))))
        return MovRegisterToRegister;
    if (checkIfImmediateMov(inputBits))
        return MovImmediateToRegister;

    // ADD
    if (checkSixBitsInRegister(inputBits, std::bitset<6>(std::string("000000"))))
        return AddRegisterToRegister;
    if (checkSevenBitsInRegister(inputBits, std::bitset<7>(std::string("0000010"))))
        return AddImmediateToAccumulator;

    // SUB
    if (checkSixBitsInRegister(inputBits, std::bitset<6>(std::string("001010"))))
        return SubRegMemoryAndRegToEither;
    if (checkSevenBitsInRegister(inputBits, std::bitset<7>(std::string("0010110"))))
        return SubImmediateFromAccumulator;

    // CMP
    if (checkSixBitsInRegister(inputBits, std::bitset<6>(std::string("001110"))))
        return CmpRegisterMemoryAndRegister;
    if (checkSevenBitsInRegister(inputBits, std::bitset<7>(std::string("0011110"))))
        return CmpImmediateWithAccumulator;

    // Common for add, sub, cmp
    if (checkSixBitsInRegister(inputBits, std::bitset<6>(std::string("100000"))))
        return XImmediateToRegisterOrMemory; // need to check second byte -> could be add, sub, cmp

    if (checkIfJump(inputBits))
        return JumpInstruction;

    return NotFound;
}

int readBinFile(const std::string &listingXAssembledPath, bool littleEndian) {
    std::ifstream inputFile(listingXAssembledPath, std::ios::binary);
    if (!inputFile)
    {
        std::cerr << "Could not open file." << std::endl;
        return 1;
    }

    uint16_t twoBytes;
    std::bitset<16> binaryTwoBytes;
    X8086Instruction instruction;
    TwoBytes sixteenBits{};

    while (inputFile.read(reinterpret_cast<char*>(&twoBytes), sizeof(twoBytes))) {
        sixteenBits = getSixteenBits(littleEndian, twoBytes, binaryTwoBytes, sixteenBits);
        instruction.operation = getOperation(sixteenBits);
        //std::cout << sixteenBits.firstByte << " " << sixteenBits.secondByte << std::endl;

        switch (instruction.operation) {
            case MovRegisterToRegister:
                outputRegToReg(sixteenBits, inputFile, instruction, "mov");
                break;
            case AddRegisterToRegister:
                outputRegToReg(sixteenBits, inputFile, instruction, "add");
                break;
            case SubRegMemoryAndRegToEither:
                outputRegToReg(sixteenBits, inputFile, instruction, "sub");
                break;
            case CmpRegisterMemoryAndRegister:
                outputRegToReg(sixteenBits, inputFile, instruction, "cmp");
                break;
            case MovImmediateToRegister:
                outputImmediateToReg(sixteenBits, inputFile, instruction, "mov");
                break;
            case AddImmediateToAccumulator:
                decodeImmediateToAcc(sixteenBits, inputFile, instruction, "add");
                break;
            case SubImmediateFromAccumulator:
                decodeImmediateToAcc(sixteenBits, inputFile, instruction, "sub");
                break;
            case CmpImmediateWithAccumulator:
                decodeImmediateToAcc(sixteenBits, inputFile, instruction, "cmp");
                break;
            case JumpInstruction:
                instruction = decodeJumpInstruction(instruction, sixteenBits);
                break;
            case XImmediateToRegisterOrMemory:
                decodeImmediateInstruction(sixteenBits, inputFile, instruction);
                break;
            default:
                std::cerr << "Operation was not found..." << std::endl;
                break;
        }
    }

    inputFile.close();

    return 0;
}

int main(int argc, char *argv[])
{
    bool littleEndian = true;
    std::string assembledPath = argv[1];
    if(readBinFile(assembledPath, littleEndian) == 1) { return 1;}
}