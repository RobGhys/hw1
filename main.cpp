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
    MovImmediateToRegister
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

bool checkIfRegToRegMov(const TwoBytes &inputBits);
bool checkIfImmediateMov(const TwoBytes &inputBits);
std::string readExtraBytes(std::ifstream &inputFile, int bytesToRead);
int decodeImmediateToRegInstruction(const TwoBytes &inputBits, X8086Instruction &instruction);

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

OperationName getOperation(const TwoBytes &inputBits) {
    OperationName result{};

    bool regToRegMov = checkIfRegToRegMov(inputBits);
    if (regToRegMov) {
        result = MovRegisterToRegister;
    } else {
        bool immediateToRegMov = checkIfImmediateMov(inputBits);
        if (immediateToRegMov) {
            result = MovImmediateToRegister;
        }
    }

    return result;
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
        std::cout << "This is a 1011" << std::endl;

    }
    return result;
}

bool checkIfRegToRegMov(const TwoBytes &inputBits) {
    std::bitset<6> movBits(std::string("100010"));
    size_t lastSixBitsSize = 6; // last from right to left
    bool result = true;
    for (int i = 0; i < lastSixBitsSize; ++i)
    {
        // i+2 because movBits is bitset<6> whereas inputBits.firstByte is bitset<8>
        if (movBits[i] != inputBits.firstByte[i+2]) {
            result = false;
            break;
        }
    }
    //std::cout << "This is a 100010" << std::endl;
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

int readBinFile(const std::string &listing37AssembledPath, bool littleEndian) {
    std::ifstream inputFile(listing37AssembledPath, std::ios::binary);
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
            int additionalBytesNb = decodeExtraBytesFromMod(sixteenBits, instruction);
            std::string byteDisplacement;
            if (instruction.operationMod == MemoryModeNoDisplacement || instruction.operationMod == MemoryMode16Bit || instruction.operationMod == MemoryMode8Bit)
                byteDisplacement = readExtraBytes(inputFile, additionalBytesNb);
            decodeRegToRegMovInstruction(sixteenBits, instruction, byteDisplacement);

            std::string instrName = "mov";
            std::cout << instrName << " " << instruction.destReg << ", " << instruction.sourceReg << std::endl;
        } else { // case for 'MovImmediateToRegister, 1011 w reg'
            int nbBytesToRead = decodeImmediateToRegInstruction(sixteenBits, instruction);
            readExtraBytes(inputFile, nbBytesToRead);
        }
    }

    inputFile.close();

    return 0;
}

int decodeImmediateToRegInstruction(const TwoBytes &inputBits, X8086Instruction &instruction) {
    int result;
    instruction.wBit = inputBits.firstByte[3];

    if (instruction.wBit == 0)
        result = 1;
    else
        result = 2;
    // fill in the bits for the 'reg' field
    for (size_t i = 0; i < 3; ++i) {
        instruction.destReg[i] = inputBits.firstByte[i];
    }

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

/*    std::cout << "--- File 37 ---" << std::endl;
    if(readBinFile(listing37AssembledPath, littleEndian) == 1) { return 1;}
    std::cout << "--- File 38 ---" << std::endl;
    if(readBinFile(listing38AssembledPath, littleEndian) == 1) { return 1;}*/
    std::cout << "--- File 39 ---" << std::endl;
    if(readBinFile(listing39AssembledPath, littleEndian) == 1) { return 1;}
}