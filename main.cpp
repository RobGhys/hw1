#include <iostream>
#include <fstream>
#include <string>
#include <bitset>

#include "instructionDecoding.h"
#include "byteReader.h"
#include "registerState.h"


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

std::vector<std::string> readBinFile(const std::string &listingXAssembledPath, bool littleEndian) {
    std::vector<std::string> instructionPrinter;

    std::ifstream inputFile(listingXAssembledPath, std::ios::binary);
    if (!inputFile)
    {
        std::cerr << "Could not open file." << std::endl;
        return instructionPrinter;
    }

    auto registerValueMap = initializeRegisterValueMap();

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
                instructionPrinter.emplace_back(outputRegToReg(sixteenBits, inputFile, instruction, "mov", registerValueMap));
                break;
            case AddRegisterToRegister:
                instructionPrinter.emplace_back(outputRegToReg(sixteenBits, inputFile, instruction, "add", registerValueMap));
                break;
            case SubRegMemoryAndRegToEither:
                instructionPrinter.emplace_back(outputRegToReg(sixteenBits, inputFile, instruction, "sub", registerValueMap));
                break;
            case CmpRegisterMemoryAndRegister:
                instructionPrinter.emplace_back(outputRegToReg(sixteenBits, inputFile, instruction, "cmp", registerValueMap));
                break;
            case MovImmediateToRegister:
                instructionPrinter.emplace_back(outputImmediateToReg(sixteenBits, inputFile, instruction, "mov", registerValueMap));
                break;
            case AddImmediateToAccumulator:
                instructionPrinter.emplace_back(decodeImmediateToAcc(sixteenBits, inputFile, instruction, "add", registerValueMap));
                break;
            case SubImmediateFromAccumulator:
                instructionPrinter.emplace_back(decodeImmediateToAcc(sixteenBits, inputFile, instruction, "sub", registerValueMap));
                break;
            case CmpImmediateWithAccumulator:
                instructionPrinter.emplace_back(decodeImmediateToAcc(sixteenBits, inputFile, instruction, "cmp", registerValueMap));
                break;
            case JumpInstruction:
                instructionPrinter.emplace_back(decodeJumpInstruction(instruction, sixteenBits));
                break;
            case XImmediateToRegisterOrMemory:
                instructionPrinter.emplace_back(decodeImmediateInstruction(sixteenBits, inputFile, instruction));
                break;
            default:
                std::cerr << "Operation was not found..." << std::endl;
                break;
        }
    }

    inputFile.close();

    // print final state of registers
    printRegisterValueMap(registerValueMap);

    return instructionPrinter;
}

int main(int argc, char *argv[])
{
    bool littleEndian = true;
    std::string assembledPath = argv[1];
    std::vector<std::string> instructionsVector = readBinFile(assembledPath, littleEndian);

    for (const std::string& instruction : instructionsVector) {
        std::cout << instruction << std::endl;
    }

}