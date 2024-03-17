#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include <vector>
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

struct BitsetHash {
    template <size_t N> // N allows for any size of bitset<>
    std::size_t operator()(const std::bitset<N>& bitset) const { // the '()' operator. const since the function doesn't modify the state of the BitsetHash
        return std::hash<unsigned long long>()(bitset.to_ullong()); // <unsigned long long> is needed because hash doesn't support bitset by default
    }
};

bool checkSixBitsInRegister(const TwoBytes &inputBits, const std::bitset<6> &instructionBits);
bool checkSevenBitsInRegister(const TwoBytes &inputBits, const std::bitset<7> &instructionBits);
bool checkIfImmediateMov(const TwoBytes &inputBits);
std::string readExtraBytes(std::ifstream &inputFile, int bytesToRead);
bool decodeImmediateToRegInstruction(const TwoBytes &inputBits, X8086Instruction &instruction);
std::bitset<8> readExtraByte(std::ifstream &inputFile);

bool checkIfJump(const TwoBytes &inputBits);

std::vector<std::bitset<8>> getJumpInstructionBytes();

bool checkAddRegToReg(const TwoBytes &bytes);

void outputRegToReg(const TwoBytes &sixteenBits, std::ifstream &inputFile, X8086Instruction &instruction, const std::string& instructionType);

void outputImmediateToReg(const TwoBytes &sixteenBits, std::ifstream &inputFile, X8086Instruction &instruction, const std::string &instructionType);

std::string readExtraBytesImmediate(std::ifstream &inputFile, int bytesToRead);

std::unordered_map<std::bitset<2>, OperationMod, BitsetHash> getMODFieldEncoding() {
    std::unordered_map<std::bitset<2>, OperationMod, BitsetHash> result;

    result[std::bitset<2>("00")] = MemoryModeNoDisplacement;
    result[std::bitset<2>("01")] = MemoryMode8Bit;
    result[std::bitset<2>("10")] = MemoryMode16Bit;
    result[std::bitset<2>("11")] = RegisterMode;

    return result;
}

std::unordered_map<std::bitset<3>, std::string, BitsetHash> getHashValuesRegisterFieldEncoding(int wBit) {
    std::unordered_map<std::bitset<3>, std::string, BitsetHash> result;

    if (wBit == 0) {
        result[std::bitset<3>("000")] = "al";
        result[std::bitset<3>("001")] = "cl";
        result[std::bitset<3>("010")] = "dl";
        result[std::bitset<3>("011")] = "bl";
        result[std::bitset<3>("100")] = "ah";
        result[std::bitset<3>("101")] = "ch";
        result[std::bitset<3>("110")] = "dh";
        result[std::bitset<3>("111")] = "bh";
    } else {
        result[std::bitset<3>("000")] = "ax";
        result[std::bitset<3>("001")] = "cx";
        result[std::bitset<3>("010")] = "dx";
        result[std::bitset<3>("011")] = "bx";
        result[std::bitset<3>("100")] = "sp";
        result[std::bitset<3>("101")] = "bp";
        result[std::bitset<3>("110")] = "si";
        result[std::bitset<3>("111")] = "di";
    }

    return result;
}

std::unordered_map<std::bitset<3>, std::string, BitsetHash> getHashEffAddCalculationFieldEncoding(OperationMod operationMod, const std::string &bitDisplacement) {
    std::unordered_map<std::bitset<3>, std::string, BitsetHash> result;

    // Common for all 3 MOD
    result[std::bitset<3>("000")] = "[bx + si]";
    result[std::bitset<3>("001")] = "[bx + di]";
    result[std::bitset<3>("010")] = "[bp + si]";
    result[std::bitset<3>("011")] = "[bp + di]";
    result[std::bitset<3>("100")] = "[si]";
    result[std::bitset<3>("101")] = "[di]";
    result[std::bitset<3>("110")] = "[bp]";
    result[std::bitset<3>("111")] = "[bx]";

    // Add 8 or 16 bit displacement
    if (operationMod == MemoryMode8Bit || operationMod == MemoryMode16Bit) {
        for (auto &[key, value]: result) {
            if (value != "[bp]") {
                auto pos = value.find_last_of(']');
                if (pos != std::string::npos) { // make sure object was found
                    value.insert(pos, " + " + bitDisplacement);
                }
            }
        }
    }

    return result;
}

std::unordered_map<std::bitset<8>, std::string, BitsetHash> getHashJumpEncoding() {
    std::unordered_map<std::bitset<8>, std::string, BitsetHash> result;

    result[std::bitset<8>("01110101")] = "jnz";
    result[std::bitset<8>("01110100")] = "je";
    result[std::bitset<8>("01111100")] = "jl";
    result[std::bitset<8>("01111110")] = "jle";
    result[std::bitset<8>("01110010")] = "jb";
    result[std::bitset<8>("01110110")] = "jbe";
    result[std::bitset<8>("01111010")] = "jp";
    result[std::bitset<8>("01110000")] = "jo";
    result[std::bitset<8>("01111000")] = "js";
    result[std::bitset<8>("01111101")] = "jnl";
    result[std::bitset<8>("01111111")] = "jg";
    result[std::bitset<8>("01110011")] = "jnb";
    result[std::bitset<8>("01110111")] = "ja";
    result[std::bitset<8>("01111011")] = "jnp";
    result[std::bitset<8>("01110001")] = "jno";
    result[std::bitset<8>("01111001")] = "jns";
    result[std::bitset<8>("11100001")] = "loop";
    result[std::bitset<8>("11100001")] = "loopz";
    result[std::bitset<8>("11100000")] = "loopnz";
    result[std::bitset<8>("11100011")] = "jcxz";

    return result;
}

std::unordered_map<std::bitset<3>, std::string, BitsetHash> getHashImmediateModEncoding() {
    std::unordered_map<std::bitset<3>, std::string, BitsetHash> result;

    result[std::bitset<3>("000")] = "add";
    result[std::bitset<3>("101")] = "sub";
    result[std::bitset<3>("111")] = "cmp";

    return result;
}

OperationName getOperation(const TwoBytes &inputBits) {
    // MOV
    if (checkSixBitsInRegister(inputBits, std::bitset<6>(std::string("100010"))))
        return MovRegisterToRegister;
    if (checkIfImmediateMov(inputBits))
        return MovImmediateToRegister;
    if (checkIfJump(inputBits))
        return JumpInstruction;

    // ADD
    if (checkSixBitsInRegister(inputBits, std::bitset<6>(std::string("000000"))))
        return AddRegisterToRegister;
    if (checkSevenBitsInRegister(inputBits, std::bitset<7>(std::string("1000010"))))
        return AddImmediateToAccumulator;

    // SUB
    if (checkSixBitsInRegister(inputBits, std::bitset<6>(std::string("001010"))))
        return SubRegMemoryAndRegToEither;
    if (checkSevenBitsInRegister(inputBits, std::bitset<7>(std::string("0010110"))))
        return SubImmediateFromAccumulator;

    // DEC
    if (checkSixBitsInRegister(inputBits, std::bitset<6>(std::string("001110"))))
        return CmpRegisterMemoryAndRegister;
    if (checkSevenBitsInRegister(inputBits, std::bitset<7>(std::string("0011110"))))
        return CmpImmediateWithAccumulator;

    // Common for add, sub, dec
    if (checkSixBitsInRegister(inputBits, std::bitset<6>(std::string("100000"))))
        return XImmediateToRegisterOrMemory; // need to check second byte -> could be add, sub, cmp

    return NotFound;
}



bool checkIfJump(const TwoBytes &inputBits) {
    std::vector<std::bitset<8>> jumpBits = getJumpInstructionBytes();

    size_t lastSixBitsSize = 8; // last from right to left
    size_t nbJumpInstructions = jumpBits.size();
    for (int j = 0; j < nbJumpInstructions; ++j) {
        bool result = true;
        for (int i = 0; i < lastSixBitsSize; ++i)
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

    //std::cout << "This was NOT a jump" << std::endl;
    return false;
}

std::vector<std::bitset<8>> getJumpInstructionBytes() {
    std::vector<std::bitset<8>> jumpBits;

    jumpBits.emplace_back("01110101"); // jnz
    jumpBits.emplace_back("01110100"); // je
    jumpBits.emplace_back("01111100"); // jl
    jumpBits.emplace_back("01111110"); // jle
    jumpBits.emplace_back("01110010"); // jb
    jumpBits.emplace_back("01110110"); // jbe
    jumpBits.emplace_back("01111010"); // jp
    jumpBits.emplace_back("01110000"); // jo
    jumpBits.emplace_back("01111000"); // js
    jumpBits.emplace_back("01111101"); // jnl
    jumpBits.emplace_back("01111111"); // jg
    jumpBits.emplace_back("01110011"); // jnb
    jumpBits.emplace_back("01110111"); // ja
    jumpBits.emplace_back("01111011"); // jnp
    jumpBits.emplace_back("01110001"); // jno
    jumpBits.emplace_back("01111001"); // jns
    jumpBits.emplace_back("11100001"); // loop
    jumpBits.emplace_back("11100001"); // loopz
    jumpBits.emplace_back("11100000"); // loopnz
    jumpBits.emplace_back("11100011"); // jcxz

    return jumpBits;
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

bool checkSixBitsInRegister(const TwoBytes &inputBits, const std::bitset<6> &instructionBits) {
    size_t lastSixBitsSize = 6; // last from right to left
    bool result = true;
    for (int i = 0; i < lastSixBitsSize; ++i)
    {
        // i+2 because instructionBits is bitset<6> whereas inputBits.firstByte is bitset<8>
        if (instructionBits[i] != inputBits.firstByte[i + 2]) {
            result = false;
            break;
        }
    }
    //std::cout << "This is a 100010" << std::endl;
    return result;
}

bool checkSevenBitsInRegister(const TwoBytes &inputBits, const std::bitset<7> &instructionBits) {
    size_t lastBitsSize = 7; // last from right to left
    bool result = true;
    for (int i = 0; i < lastBitsSize; ++i)
    {
        // i+1 because instructionBits is bitset<7> whereas inputBits.firstByte is bitset<8>
        if (instructionBits[i] != inputBits.firstByte[i + 1]) {
            result = false;
            break;
        }
    }
    return result;
}

int decodeExtraBytesFromMod(const TwoBytes &inputBits, X8086Instruction &instruction) {

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

unsigned int convertOneByteBase2ToBase10(const std::bitset<8> &secondByte) {
    unsigned int result = 0;
    for (size_t i = 0; i < 8; ++i) {
        if (secondByte[i]) {
            result += 1 << i;
        }
    }

    return result;
}

unsigned int convertTwoByteBases2ToBase10(const std::bitset<16> &bytes) {
    unsigned int result = 0;
    for (size_t i = 0; i < 16; ++i) {
        if (bytes[i]) {
            result += 1 << i;
        }
    }

    return result;
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
        binaryTwoBytes = std::bitset<16>(twoBytes); // we must read at least 2 bytes per instruction. Up to 4.
        for (int i = 0; i < 8; ++i)
        {
            if (littleEndian)
            {
                sixteenBits.firstByte[i] = binaryTwoBytes[i];
                sixteenBits.secondByte[i] = binaryTwoBytes[i+8];
            } else {
                std::cout << "Big endian" << std::endl;
                sixteenBits.firstByte[i] = binaryTwoBytes[i+8];
                sixteenBits.secondByte[i] = binaryTwoBytes[i];
            }
        }
        instruction.operation = getOperation(sixteenBits);

        if (instruction.operation == MovRegisterToRegister) { // 100010dw
            outputRegToReg(sixteenBits, inputFile, instruction, "mov");
        } else if (instruction.operation == AddRegisterToRegister) {
            outputRegToReg(sixteenBits, inputFile, instruction, "add");
        } else if (instruction.operation == SubRegMemoryAndRegToEither) {
            outputRegToReg(sixteenBits, inputFile, instruction, "sub");
        } else if (instruction.operation == CmpRegisterMemoryAndRegister) {
            outputRegToReg(sixteenBits, inputFile, instruction, "cmp");
        } else if (instruction.operation == MovImmediateToRegister) { // case for 'MovImmediateToRegister, 1011 w reg'
            outputImmediateToReg(sixteenBits, inputFile, instruction, "mov");
        } else if (instruction.operation == AddImmediateToAccumulator) {
            outputImmediateToReg(sixteenBits, inputFile, instruction, "add");
        } else if (instruction.operation == SubImmediateFromAccumulator) {
            outputImmediateToReg(sixteenBits, inputFile, instruction, "sub");
        } else if (instruction.operation == CmpImmediateWithAccumulator) {
            outputImmediateToReg(sixteenBits, inputFile, instruction, "cmp");
        } else if (instruction.operation == JumpInstruction) {
            auto hashJumpEncoding = getHashJumpEncoding();
            std::string instrName = hashJumpEncoding[sixteenBits.firstByte];
            instruction.sourceReg = std::to_string(convertOneByteBase2ToBase10(sixteenBits.secondByte));
        } else if (instruction.operation == XImmediateToRegisterOrMemory) {
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

            auto operationTypeHashMap = getHashImmediateModEncoding();
            auto operationType = operationTypeHashMap[operationField];
            std::cout << "Operation type : " << operationType << std::endl;


            // Get value of 'r/m' register
            int additionalBytesNb = decodeExtraBytesFromMod(sixteenBits, instruction);
            std::string byteDisplacement;
            if (instruction.operationMod == MemoryModeNoDisplacement || instruction.operationMod == MemoryMode16Bit || instruction.operationMod == MemoryMode8Bit)
                byteDisplacement = readExtraBytes(inputFile, additionalBytesNb);

            std::unordered_map<std::bitset<3>, std::string, BitsetHash>
                    registerBitsetMap = getHashValuesRegisterFieldEncoding(instruction.wBit);

            // Register to register (MOD 11)
            if (instruction.operationMod == RegisterMode) {
                    instruction.destReg = registerBitsetMap[rmField];
            } else { // Mod is 00, 01 or 10
                std::unordered_map<std::bitset<3>, std::string, BitsetHash>
                        effectiveAddressMap = getHashEffAddCalculationFieldEncoding(instruction.operationMod, byteDisplacement);
                    instruction.destReg = effectiveAddressMap[rmField];
            }

            // Read source address
            int dataByte = 1; // for 'data'
            if (instruction.sBit == 0 && instruction.wBit == 1)
                dataByte += 1; // another 'data' byte

            std::cout << "Total bytes to read: " << dataByte << std::endl;
            auto dataByteString = readExtraBytesImmediate(inputFile, dataByte);

            std::string output = operationType + " " +instruction.destReg + ", " + dataByteString;
            std::cout << output << std::endl;
        } else { // "1000000"
            std::cerr << "Operate was not found..." << std::endl;
        }
    }

    inputFile.close();

    return 0;
}

std::string readExtraBytesImmediate(std::ifstream &inputFile, int bytesToRead) {
    int byteDisplacement = 0;
    unsigned char additionalBytes[2];
    inputFile.read(reinterpret_cast<char*>(additionalBytes), bytesToRead);

    if (bytesToRead == 1) {
        byteDisplacement = additionalBytes[0];
        //std::cout << "displacement -> " << byteDisplacement << std::endl;
    } else if (bytesToRead == 2) {
        // endian order matters. Little endian -> most significant byte is read 2nd
        byteDisplacement = additionalBytes[0] | (additionalBytes[1] << 8);
    }

    return std::to_string(byteDisplacement);
}

void outputImmediateToReg(const TwoBytes &sixteenBits, std::ifstream &inputFile,
                          X8086Instruction &instruction, const std::string &instructionType) {
    //std::cout << "--1011--"<< std::endl;
    bool readAdditionalByte = decodeImmediateToRegInstruction(sixteenBits, instruction);

    //std::cout << "additional byte? : " << readAdditionalByte << std::endl;
    if (!readAdditionalByte) {
        instruction.sourceReg = std::to_string(convertOneByteBase2ToBase10(sixteenBits.secondByte));
    } else {
        std::bitset<8> highByte = readExtraByte(inputFile);
        std::bitset<16> combinedBytes;
        for (size_t i = 0; i < 8; ++i) {
            combinedBytes[i] = sixteenBits.secondByte[i]; // less significant byte
            combinedBytes[i + 8] = highByte[i]; // most significant byte
        }

        instruction.sourceReg = std::to_string(convertTwoByteBases2ToBase10(combinedBytes));
    }
    std::string output = instructionType + " " + instruction.destReg + ", " + instruction.sourceReg;
    std::cout << output << std::endl;
}

void outputRegToReg(const TwoBytes &sixteenBits, std::ifstream &inputFile, X8086Instruction &instruction, const std::string& instructionType) {
    int additionalBytesNb = decodeExtraBytesFromMod(sixteenBits, instruction);
    std::string byteDisplacement;
    if (instruction.operationMod == MemoryModeNoDisplacement || instruction.operationMod == MemoryMode16Bit || instruction.operationMod == MemoryMode8Bit)
        byteDisplacement = readExtraBytes(inputFile, additionalBytesNb);
    decodeRegToRegMovInstruction(sixteenBits, instruction, byteDisplacement);

    std::string output = instructionType + " " +instruction.destReg + ", " + instruction.sourceReg;
    std::cout << output << std::endl;
}

std::bitset<8> readExtraByte(std::ifstream &inputFile) {
    unsigned char additionalBytes[1];
    inputFile.read(reinterpret_cast<char *>(additionalBytes), 1);

    std::bitset<8> byteDisplacement(additionalBytes[0]);
    //std::cout << "displacement -> " << byteDisplacement.to_ulong() << std::endl;

    return byteDisplacement;
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


std::string readExtraBytes(std::ifstream &inputFile, int bytesToRead) {
    if (bytesToRead == 0)
        return "";

    int byteDisplacement = 0;
    if (bytesToRead > 0 && bytesToRead <= 2) {
        unsigned char additionalBytes[2];
        inputFile.read(reinterpret_cast<char*>(additionalBytes), bytesToRead);

        if (bytesToRead == 1) {
            byteDisplacement = additionalBytes[0];
            //std::cout << "displacement -> " << byteDisplacement << std::endl;
        } else if (bytesToRead == 2) {
            // endian order matters. Little endian -> most significant byte is read 2nd
            byteDisplacement = additionalBytes[0] | (additionalBytes[1] << 8);
        }
    }
    return std::to_string(byteDisplacement);
}

int main()
{
    bool littleEndian = true;
    std::string listing37AssembledPath = "/home/rob/Documents/Github/computer_enhance/hw1/listing_0037_single_register_mov";
    std::string listing38AssembledPath = "/home/rob/Documents/Github/computer_enhance/hw1/listing_0038_many_register_mov";
    std::string listing39AssembledPath = "/home/rob/Documents/Github/computer_enhance/hw1/listing_0039_more_movs";
    std::string listing41AssembledPath = "/home/rob/Documents/Github/computer_enhance/hw1/listing_0041_add_sub_cmp_jnz";

/*    std::cout << "--- File 37 ---" << std::endl;
    if(readBinFile(listing37AssembledPath, littleEndian) == 1) { return 1;}
    std::cout << "--- File 38 ---" << std::endl;
    if(readBinFile(listing38AssembledPath, littleEndian) == 1) { return 1;}*/
    /*std::cout << "--- File 39 ---" << std::endl;
    if(readBinFile(listing39AssembledPath, littleEndian) == 1) { return 1;}*/
    std::cout << "--- File 41 ---" << std::endl;
    if(readBinFile(listing41AssembledPath, littleEndian) == 1) { return 1;}
}