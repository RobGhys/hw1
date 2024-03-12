#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include <vector>
#include <unordered_map>

using namespace std;

struct SixteenBits {
    bitset<8> firstByte;
    bitset<8> secondByte;
};

enum OperationMod {
    twoRegisters,
    registerMemory
};

struct X8086Instruction {
    string operation; // mov
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
    string sourceReg;
    string destReg;
    OperationMod operationMod{};
};

struct BitsetHash {
    template <size_t N> // N allows for any size of bitset<>
    std::size_t operator()(const std::bitset<N>& bitset) const { // the '()' operator. const since the function doesn't modify the state of the BitsetHash
        return std::hash<unsigned long long>()(bitset.to_ullong()); // <unsigned long long> is needed because hash doesn't support bitset by default
    }
};

std::unordered_map<std::bitset<3>, std::string, BitsetHash> getHashValues(int wBit) {
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

X8086Instruction &getOperation(const SixteenBits &inputBits, X8086Instruction &result) {
    bitset<6> movBits(string("100010"));
    size_t firstSixBitsSize = 6;
    bool sameBits = true;
    for (int i = 0; i < firstSixBitsSize; ++i)
    {
        // i+2 because movBits is bitset<6> whereas inputBits.firstByte is bitset<8>
        if (movBits[i] != inputBits.firstByte[i+2]) {
            cout << "false! at i ==" << i << "movBits: " << movBits[i] << " | firstByte: " << inputBits.firstByte[i] << endl;
            sameBits = false;
            break;
        }
    }
    if (sameBits)
    {
        result.operation = "mov";
    }
    return result;
}

X8086Instruction decodeInstruction(SixteenBits inputBits) {
    X8086Instruction result;
    // note: bits are read right to left
    result = getOperation(inputBits, result);

    result.dBit = inputBits.firstByte[1];
    // right-most bit
    result.wBit = inputBits.firstByte[0];

    bitset<2> modField;
    bitset<3> regField;
    bitset<3> rmField;

    for (size_t i = 0; i < 2; ++i) {
        modField[i] = inputBits.secondByte[i + 6];
    }

    for (size_t j = 0; j < 3; ++j) {
        regField[j] = inputBits.secondByte[j + 3];
    }

    for (size_t k = 0; k < 3; ++k) {
        rmField[k] = inputBits.secondByte[k];
    }

    std::unordered_map<std::bitset<3>, std::string, BitsetHash> bitsetMap = getHashValues(result.wBit);

    if (result.dBit == 0) {
        result.sourceReg = bitsetMap[regField];
        result.destReg = bitsetMap[rmField];
    } else {
        result.sourceReg = bitsetMap[rmField];
        result.destReg = bitsetMap[regField];
    }

    return result;
}

int readBinFile(const string &listing37AssembledPath, bool littleEndian) {
    ifstream inputFile(listing37AssembledPath, ios::binary);
    if (!inputFile)
    {
        cerr << "Could not open file." << endl;
        return 1;
    }

    vector<SixteenBits> sixteenBitsVector;
    uint16_t twoBytes;

    while (inputFile.read(reinterpret_cast<char*>(&twoBytes), sizeof(twoBytes)))
    {
        auto binaryTwoBytes = bitset<16>(twoBytes);

        SixteenBits sixteenBits;
        for (int i = 0; i < 8; ++i)
        {
            if (littleEndian)
            {
                sixteenBits.firstByte[i] = binaryTwoBytes[i];
                sixteenBits.secondByte[i] = binaryTwoBytes[i+8];
            } else {
                cout << "Big endian" << endl;
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
        cout << instruction.operation << " " << instruction.destReg << ", " << instruction.sourceReg << endl;
    }

    inputFile.close();

    return 0;
}

int main()
{
    bool littleEndian = true;
    string listing37AssembledPath = "/home/rob/Documents/Github/computer_enhance/hw1/listing_0037_single_register_mov";
    string listing38AssembledPath = "/home/rob/Documents/Github/computer_enhance/hw1/listing_0038_many_register_mov";

    if(readBinFile(listing37AssembledPath, littleEndian) == 1) { return 1;}
    cout << endl;
    if(readBinFile(listing38AssembledPath, littleEndian) == 1) { return 1;}
}