//
// Created by rob on 18/03/24.
//

#include "decodingHashMaps.h"

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
    result[std::bitset<3>("000")] = "[bx+si]";
    result[std::bitset<3>("001")] = "[bx+di]";
    result[std::bitset<3>("010")] = "[bp+si]";
    result[std::bitset<3>("011")] = "[bp+di]";
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
    result[std::bitset<8>("11100010")] = "loop";
    result[std::bitset<8>("11100001")] = "loopz";
    result[std::bitset<8>("11100000")] = "loopnz";
    result[std::bitset<8>("11100011")] = "jcxz";

    return result;
}

std::unordered_map<std::bitset<3>, std::string, BitsetHash> getHashAddSubCmpTypeEncoding() {
    std::unordered_map<std::bitset<3>, std::string, BitsetHash> result;

    result[std::bitset<3>("000")] = "add";
    result[std::bitset<3>("101")] = "sub";
    result[std::bitset<3>("111")] = "cmp";

    return result;
}