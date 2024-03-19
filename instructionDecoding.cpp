//
// Created by rob on 18/03/24.
//

#include <vector>
#include "instructionDecoding.h"
#include "decodingHashMaps.h"
#include "byteReader.h"
#include "registerState.h"

std::string decodeJumpInstruction(X8086Instruction &instruction, const TwoBytes &sixteenBits) {
    auto hashJumpEncoding = getHashJumpEncoding();
    std::string instrName = hashJumpEncoding[sixteenBits.firstByte];
    instruction.sourceReg = std::to_string(convertOneByteBase2ToBase10(sixteenBits.secondByte));
    return instrName + " " + instruction.sourceReg;
}

std::string decodeImmediateInstruction(const TwoBytes &sixteenBits, std::ifstream &inputFile, X8086Instruction &instruction) {
    instruction.sBit = sixteenBits.firstByte[1];
    // right-most bit
    instruction.wBit = sixteenBits.firstByte[0];
    std::bitset<3> operationField; // could be add, sub, or cmp
    std::bitset<3> rmField;

    for (size_t j = 0; j < 3; ++j) {
        operationField[j] = sixteenBits.secondByte[j + 3];
    }

    for (size_t k = 0; k < 3; ++k) {
        rmField[k] = sixteenBits.secondByte[k];
    }

    auto operationTypeHashMap = getHashAddSubCmpTypeEncoding();
    auto operationType = operationTypeHashMap[operationField];

    // Get value of 'r/m' register
    int additionalBytesNb = getModAndDecodeExtraBytes(sixteenBits, instruction);
    std::string byteDisplacement;
    if (instruction.operationMod == MemoryModeNoDisplacement || instruction.operationMod == MemoryMode16Bit || instruction.operationMod == MemoryMode8Bit)
        byteDisplacement = readExtraBytes(inputFile, additionalBytesNb);

    std::unordered_map<std::bitset<3>, std::string, BitsetHash>
            registerBitsetMap = getHashValuesRegisterFieldEncoding(instruction.wBit);

    std::string operationSize;

    int dataByte = 1; // for 'data'
    if (operationType == "cmp") { // cmp has condition 'data if s: w = 1'
        //std::cout << "optype 111" << std::endl;
        if (instruction.sBit == 1 && instruction.wBit == 1)
            dataByte += 1; // another 'data' byte
        //std::cout << "dataByte is: " << dataByte << std::endl;
    } else { // add and sub have condition 'data if s: w = 01'
        if (instruction.sBit == 0 && instruction.wBit == 1)
            dataByte += 1; // another 'data' byte
    }

    // Register to register (MOD 11)
    if (instruction.operationMod == RegisterMode) {
        instruction.destReg = registerBitsetMap[rmField];
    } else { // Mod is 00, 01 or 10
        if (rmField != std::bitset<3>("110")) {
            std::unordered_map<std::bitset<3>, std::string, BitsetHash>
                    effectiveAddressMap = getHashEffAddCalculationFieldEncoding(instruction.operationMod, byteDisplacement);
            instruction.destReg = effectiveAddressMap[rmField];
        } else { // DIRECT ADDRESS case
            //std::cout << "Special case r/m is 110." << std::endl;
            auto twoBytesDisplacement = readExtraBytes(inputFile, 2);
            instruction.destReg = twoBytesDisplacement;
        }

        if (instruction.wBit == 0) {
            operationSize = "byte";
        } else {
            operationSize = "word";
        }
    }

    //std::cout << "Total bytes to read: " << dataByte << std::endl;
    auto dataBytes = readDataBytes(inputFile);

    std::string output = operationType;
    if (!operationSize.empty()) {
        output += " " + operationSize + " " + instruction.destReg + ", " + dataBytes;
    } else {
        output += " " + instruction.destReg + ", " + dataBytes;

    }
    return output;
}

int getModAndDecodeExtraBytes(const TwoBytes &inputBits, X8086Instruction &instruction) {
    std::bitset<2> modField;

    for (size_t i = 0; i < 2; ++i) {
        modField[i] = inputBits.secondByte[i + 6];
    }

    // Get MOD
    std::unordered_map<std::bitset<2>, OperationMod, BitsetHash> modBitsetMap = getMODFieldEncoding();
    instruction.operationMod = modBitsetMap[modField];
    //std::cout << "Modfield -> " << modField << std::endl;

    if (instruction.operationMod == RegisterMode || instruction.operationMod == MemoryModeNoDisplacement)
        return 0;
    else if (instruction.operationMod == MemoryMode8Bit)
        return 1;
    else if (instruction.operationMod == MemoryMode16Bit)
        return 2;

    return -1; // problem
}

void outputImmediateToReg(const TwoBytes &sixteenBits, std::ifstream &inputFile,
                          X8086Instruction &instruction, const std::string &instructionType, ProgramOutput &programOutput) {
    //std::cout << "--1011--"<< std::endl;
    bool readAdditionalByte = decodeImmediateToRegInstruction(sixteenBits, instruction);

    //std::cout << "additional byte? : " << readAdditionalByte << std::endl;
    if (!readAdditionalByte) {
        instruction.sourceReg = std::to_string(convertOneByteBase2ToBase10(sixteenBits.secondByte));
        // actually do the operation on register
        updateRegisterValueMap(programOutput.registerValueMap, instruction.destReg, convertOneByteBase2ToBase10(sixteenBits.secondByte));
    } else {
        std::bitset<8> highByte = readExtraByte(inputFile);
        std::bitset<16> combinedBytes;
        for (size_t i = 0; i < 8; ++i) {
            combinedBytes[i] = sixteenBits.secondByte[i]; // less significant byte
            combinedBytes[i + 8] = highByte[i]; // most significant byte
        }

        instruction.sourceReg = std::to_string(convertTwoByteBases2ToBase10(combinedBytes));
        // actually do the operation on register
        updateRegisterValueMap(programOutput.registerValueMap, instruction.destReg, convertTwoByteBases2ToBase10(combinedBytes));

    }

    programOutput.instructionPrinter.emplace_back(instructionType + " " + instruction.destReg + ", " + instruction.sourceReg);
}

void outputRegToReg(const TwoBytes &sixteenBits, std::ifstream &inputFile, X8086Instruction &instruction, const std::string& instructionType, ProgramOutput &programOutput) {
    int additionalBytesNb = getModAndDecodeExtraBytes(sixteenBits, instruction);
    std::string byteDisplacement;
    if (instruction.operationMod == MemoryModeNoDisplacement || instruction.operationMod == MemoryMode16Bit || instruction.operationMod == MemoryMode8Bit)
        byteDisplacement = readExtraBytes(inputFile, additionalBytesNb);
    decodeRegToRegMovInstruction(sixteenBits, instruction, byteDisplacement);

    // actually do the operation on register
    int valueOfSourceReg = programOutput.registerValueMap[instruction.sourceReg];
    updateRegisterValueMap(programOutput.registerValueMap, instruction.destReg, valueOfSourceReg);

    programOutput.instructionPrinter.emplace_back(instructionType + " " + instruction.destReg + ", " + instruction.sourceReg);
}

bool decodeImmediateToRegInstruction(const TwoBytes &inputBits, X8086Instruction &instruction) {
    int result;
    instruction.wBit = inputBits.firstByte[3];

    if (instruction.wBit == 0)
        result = false;
    else
        result = true;

    std::bitset<3> rmField;

    for (size_t i = 0; i < 3; ++i) {
        rmField[i] = inputBits.firstByte[i];
    }

    // Get value from mapping
    std::unordered_map<std::bitset<3>, std::string, BitsetHash>
            registerBitsetMap = getHashValuesRegisterFieldEncoding(instruction.wBit);
    instruction.destReg = registerBitsetMap[rmField];

    return result;
}

void decodeRegToRegMovInstruction(const TwoBytes &inputBits, X8086Instruction &instruction,
                                  const std::string& byteDisplacement) {
    instruction.dBit = inputBits.firstByte[1];
    // right-most bit
    instruction.wBit = inputBits.firstByte[0];

    std::bitset<3> regField;
    std::bitset<3> rmField;

    for (size_t j = 0; j < 3; ++j) {
        regField[j] = inputBits.secondByte[j + 3];
    }

    for (size_t k = 0; k < 3; ++k) {
        rmField[k] = inputBits.secondByte[k];
    }

    // Get source/dest Registers
    std::unordered_map<std::bitset<3>, std::string, BitsetHash>
            registerBitsetMap = getHashValuesRegisterFieldEncoding(instruction.wBit);

    // Register to register (MOD 11)
    if (instruction.operationMod == RegisterMode) {
        if (instruction.dBit == 0) {
            instruction.sourceReg = registerBitsetMap[regField];
            instruction.destReg = registerBitsetMap[rmField];
        } else {
            instruction.sourceReg = registerBitsetMap[rmField];
            instruction.destReg = registerBitsetMap[regField];
        }
    } else { // Mod is 00, 01 or 10
        std::unordered_map<std::bitset<3>, std::string, BitsetHash>
                effectiveAddressMap = getHashEffAddCalculationFieldEncoding(instruction.operationMod, byteDisplacement);
        if (instruction.dBit == 0) {
            instruction.sourceReg = registerBitsetMap[regField];
            instruction.destReg = effectiveAddressMap[rmField];
        } else {
            instruction.sourceReg = effectiveAddressMap[rmField];
            instruction.destReg = registerBitsetMap[regField];
        }
    }
}

void decodeImmediateToAcc(const TwoBytes &sixteenBits, std::ifstream &inputFile, X8086Instruction &instruction,
                                 const std::string &operationType, ProgramOutput &programOutput) {
    instruction.wBit = sixteenBits.firstByte[0];
    if (instruction.wBit == 0) {
        instruction.destReg = "al";
        instruction.sourceReg = std::to_string(convertOneByteBase2ToBase10(sixteenBits.secondByte));
    } else {
        instruction.destReg = "ax";

        std::bitset<8> highByte = readExtraByte(inputFile);
        std::bitset<16> combinedBytes;
        for (size_t i = 0; i < 8; ++i) {
            combinedBytes[i] = sixteenBits.secondByte[i]; // less significant byte
            combinedBytes[i + 8] = highByte[i]; // most significant byte
        }

        instruction.sourceReg = std::to_string(convertTwoByteBases2ToBase10(combinedBytes));
    }
    programOutput.instructionPrinter.emplace_back(operationType + " " + instruction.destReg + ", " + instruction.sourceReg);
}

bool checkIfJump(const TwoBytes &inputBits) {
    std::vector<std::bitset<8>> jumpBits = getJumpInstructionBytes();

    size_t lastBitsSize = 8; // last from right to left
    size_t nbJumpInstructions = jumpBits.size();
    for (int j = 0; j < nbJumpInstructions; ++j) {
        bool result = true;
        for (int i = 0; i < lastBitsSize; ++i)
        {
            if (jumpBits[j][i] != inputBits.firstByte[i]) {
                result = false;
                break;
            }
        }
        if (result) {
            return result;
        }
    }

    return false;
}

bool checkIfImmediateMov(const TwoBytes &inputBits) {
    //std::cout << "input bits -> " << inputBits.firstByte << std::endl;
    bool result = true;
    std::bitset<4> bitsImmediateToReg(std::string("1011"));
    size_t lastFourBitsSize = 4; // last from right to left
    for (int i = 0; i < lastFourBitsSize; ++i) {
        //std::cout << "bit: " << inputBits.firstByte[i+4] << ", ";
        // i+4 because movBits is bitset<6> whereas inputBits.firstByte is bitset<8>
        if (bitsImmediateToReg[i] != inputBits.firstByte[i+4]) {
            //std::cout << "Not -- 1011 --" << std::endl;
            result = false;
            break;
        }
    }
    //std::cout << "This is a 1011" << std::endl;

    return result;
}