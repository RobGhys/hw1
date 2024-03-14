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
    registerMode,
    memoryModeNoDisplacement,
    memoryMode8bit,
    memoryMode16bit
};

struct X8086Instruction {
    std::string operation; // mov
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

std::unordered_map<std::bitset<2>, OperationMod, BitsetHash> getMODFieldEncoding() {
    std::unordered_map<std::bitset<2>, OperationMod, BitsetHash> result;

    result[std::bitset<2>("00")] = memoryModeNoDisplacement;
    result[std::bitset<2>("01")] = memoryMode8bit;
    result[std::bitset<2>("10")] = memoryMode16bit;
    result[std::bitset<2>("11")] = registerMode;

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

X8086Instruction &getOperation(const TwoBytes &inputBits, X8086Instruction &result) {
    std::bitset<6> movBits(std::string("100010"));
    size_t firstSixBitsSize = 6;
    bool sameBits = true;
    for (int i = 0; i < firstSixBitsSize; ++i)
    {
        // i+2 because movBits is bitset<6> whereas inputBits.firstByte is bitset<8>
        if (movBits[i] != inputBits.firstByte[i+2]) {
            std::cout << "false! at i ==" << i << "movBits: " << movBits[i] << " | firstByte: " << inputBits.firstByte[i] << std::endl;
            sameBits = false;
            break;
        }
    }
    if (sameBits)
    {
        result.operation = "mov";
    }
}

X8086Instruction decodeInstruction(TwoBytes inputBits) {
    X8086Instruction result;
    // note: bits are read right to left
    getOperation(inputBits, result);

    // Mov case
    if (result.operation == "mov") {
        result.dBit = inputBits.firstByte[1];
        // right-most bit
        result.wBit = inputBits.firstByte[0];

        std::bitset<2> modField;
        std::bitset<3> regField;
        std::bitset<3> rmField;

        for (size_t i = 0; i < 2; ++i) {
            modField[i] = inputBits.secondByte[i + 6];
        }

        for (size_t j = 0; j < 3; ++j) {
            regField[j] = inputBits.secondByte[j + 3];
        }

        for (size_t k = 0; k < 3; ++k) {
            rmField[k] = inputBits.secondByte[k];
        }

        // Get MOD
        std::unordered_map<std::bitset<2>, OperationMod, BitsetHash> modBitsetMap = getMODFieldEncoding();
        result.operationMod = modBitsetMap[modField];

        // Register to register (MOD 11)
        if (result.operationMod == registerMode) {
            // Get source/dest Registers
            std::unordered_map<std::bitset<3>, std::string, BitsetHash> registerBitsetMap = getHashValuesRegisterFieldEncoding(
                    result.wBit);

            if (result.dBit == 0) {
                result.sourceReg = registerBitsetMap[regField];
                result.destReg = registerBitsetMap[rmField];
            } else {
                result.sourceReg = registerBitsetMap[rmField];
                result.destReg = registerBitsetMap[regField];
            }
        }
    }

    return result;
}

int readBinFile(const std::string &listing37AssembledPath, bool littleEndian) {
    std::ifstream inputFile(listing37AssembledPath, std::ios::binary);
    if (!inputFile)
    {
        std::cerr << "Could not open file." << std::endl;
        return 1;
    }

    std::vector<TwoBytes> sixteenBitsVector;
    uint16_t twoBytes;

    while (inputFile.read(reinterpret_cast<char*>(&twoBytes), sizeof(twoBytes)))
    {
        auto binaryTwoBytes = std::bitset<16>(twoBytes);

        TwoBytes sixteenBits;
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
        sixteenBitsVector.push_back(sixteenBits);
    }
    size_t nb_elem = sixteenBitsVector.size();
    for (int i = 0; i < nb_elem; ++i)
    {
        X8086Instruction instruction = decodeInstruction(sixteenBitsVector[i]);
        std::cout << instruction.operation << " " << instruction.destReg << ", " << instruction.sourceReg << std::endl;
    }

    inputFile.close();

    return 0;
}

int main()
{
    bool littleEndian = true;
    std::string listing37AssembledPath = "/home/rob/Documents/Github/computer_enhance/hw1/listing_0037_single_register_mov";
    std::string listing38AssembledPath = "/home/rob/Documents/Github/computer_enhance/hw1/listing_0038_many_register_mov";

    if(readBinFile(listing37AssembledPath, littleEndian) == 1) { return 1;}
    std::cout << std::endl;
    if(readBinFile(listing38AssembledPath, littleEndian) == 1) { return 1;}
}