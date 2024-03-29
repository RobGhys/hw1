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

void addBinaryToStringVector(std::vector<std::string>& outputVector, const TwoBytes& twoBytes) {
    std::string firstByteStr = twoBytes.firstByte.to_string();
    std::string secondByteStr = twoBytes.secondByte.to_string();

    std::string combined = firstByteStr + " " + secondByteStr;

    outputVector.push_back(combined);
}

ProgramOutput readBinFile(const std::string &listingXAssembledPath, bool littleEndian) {
    ProgramOutput programOutput;

    programOutput.instructionPrinter = std::vector<std::string>{};
    programOutput.registerValueMap =initializeRegisterValueMap();

    std::ifstream inputFile(listingXAssembledPath, std::ios::binary);
    if (!inputFile)
    {
        std::cerr << "Could not open file." << std::endl;
        return programOutput;
    }

    uint16_t twoBytes;
    std::bitset<16> binaryTwoBytes;
    TwoBytes sixteenBits{};
    InstructionPointer ip{};

    while (inputFile.read(reinterpret_cast<char*>(&twoBytes), sizeof(twoBytes))) {
        X8086Instruction instruction{};

        sixteenBits = getSixteenBits(littleEndian, twoBytes, binaryTwoBytes, sixteenBits);
        instruction.operation = getOperation(sixteenBits);
        //addBinaryToStringVector(programOutput.instructionPrinter, sixteenBits); // debugging
        //std::cout << sixteenBits.firstByte << " " << sixteenBits.secondByte << std::endl;

        switch (instruction.operation) {
            case MovRegisterToRegister:
                outputRegToReg(sixteenBits, inputFile, instruction, "mov", programOutput, ip);
                break;
            case AddRegisterToRegister:
                outputRegToReg(sixteenBits, inputFile, instruction, "add", programOutput, ip);
                break;
            case SubRegMemoryAndRegToEither:
                outputRegToReg(sixteenBits, inputFile, instruction, "sub", programOutput, ip);
                break;
            case CmpRegisterMemoryAndRegister:
                outputRegToReg(sixteenBits, inputFile, instruction, "cmp", programOutput, ip);
                break;
            case MovImmediateToRegister:
                outputImmediateToReg(sixteenBits, inputFile, instruction, "mov", programOutput, ip);
                break;
            case AddImmediateToAccumulator:
                decodeImmediateToAcc(sixteenBits, inputFile, instruction, "add", programOutput, ip);
                break;
            case SubImmediateFromAccumulator:
                decodeImmediateToAcc(sixteenBits, inputFile, instruction, "sub", programOutput, ip);
                break;
            case CmpImmediateWithAccumulator:
                decodeImmediateToAcc(sixteenBits, inputFile, instruction, "cmp", programOutput, ip);
                break;
            case JumpInstruction:
                decodeJumpInstruction(instruction, sixteenBits, programOutput, ip);
                break;
            case XImmediateToRegisterOrMemory:
                decodeImmediateInstruction(sixteenBits, inputFile, instruction, programOutput, ip);
                break;
            default:
                std::cerr << "Operation was not found : " << sixteenBits.firstByte << " " << sixteenBits.secondByte << std::endl;
                break;
        }
    }

    inputFile.close();

    programOutput.instructionPointer = ip.ip;

    return programOutput;
}

int main(int argc, char *argv[])
{
    bool littleEndian = true;
    std::string assembledPath = argv[1];

    ProgramOutput programOutput = readBinFile(assembledPath, littleEndian);
    std::cout << "\n=== Instructions ==" << std::endl;

    for (const std::string& instruction : programOutput.instructionPrinter) {
        std::cout << instruction << std::endl;
    }

    std::cout << "\n=== Registers state ==" << std::endl;
    printRegisterValueMap(programOutput.registerValueMap);

    std::cout << "\n=== Flags ===" << std::endl << "Z -> " << programOutput.flags.zeroFlag << " | S -> " << programOutput.flags.signFlag << std::endl;
    std::cout << "\n=== IP ===" << std::endl;
    showAsHexa(programOutput.instructionPointer);
}