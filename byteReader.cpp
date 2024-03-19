//
// Created by rob on 18/03/24.
//

#include "byteReader.h"
#include "instructionDecoding.h"
#include <fstream>

std::bitset<8> readExtraByte(std::ifstream &inputFile) {
    unsigned char additionalBytes[1];
    inputFile.read(reinterpret_cast<char *>(additionalBytes), 1);

    std::bitset<8> byteDisplacement(additionalBytes[0]);
    //std::cout << "displacement -> " << byteDisplacement.to_ulong() << std::endl;

    return byteDisplacement;
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

std::string readDataBytes(std::ifstream &inputFile) {
    int byteDisplacement = 0;
    unsigned char additionalBytes[2];
    inputFile.read(reinterpret_cast<char*>(additionalBytes), 1);

    byteDisplacement = additionalBytes[0];
    return std::to_string(byteDisplacement);
}

int convertOneByteBase2ToBase10(const std::bitset<8> &secondByte) {
    int result = 0;
    for (size_t i = 0; i < 8; ++i) {
        if (secondByte[i]) {
            result += 1 << i;
        }
    }

    return result;
}

int convertTwoByteBases2ToBase10(const std::bitset<16> &bytes) {
    int result = 0;
    for (size_t i = 0; i < 16; ++i) {
        if (bytes[i]) {
            result += 1 << i;
        }
    }

    return result;
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
    jumpBits.emplace_back("11100010"); // loop
    jumpBits.emplace_back("11100001"); // loopz
    jumpBits.emplace_back("11100000"); // loopnz
    jumpBits.emplace_back("11100011"); // jcxz

    return jumpBits;
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

TwoBytes &getSixteenBits(bool littleEndian, uint16_t twoBytes, std::bitset<16> &binaryTwoBytes, TwoBytes &sixteenBits) {
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
    return sixteenBits;
}