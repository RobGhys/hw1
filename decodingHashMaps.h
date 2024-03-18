//
// Created by rob on 18/03/24.
//

#ifndef HW1_DECODINGHASHMAPS_H
#define HW1_DECODINGHASHMAPS_H

#include <iostream>
#include <unordered_map>
#include <bitset>

#include "instructionDecoding.h"


struct BitsetHash {
    template <size_t N> // N allows for any size of bitset<>
    std::size_t operator()(const std::bitset<N>& bitset) const { // the '()' operator. const since the function doesn't modify the state of the BitsetHash
        return std::hash<unsigned long long>()(bitset.to_ullong()); // <unsigned long long> is needed because hash doesn't support bitset by default
    }
};

std::unordered_map<std::bitset<2>, OperationMod, BitsetHash> getMODFieldEncoding();
std::unordered_map<std::bitset<3>, std::string, BitsetHash> getHashValuesRegisterFieldEncoding(int wBit);
std::unordered_map<std::bitset<3>, std::string, BitsetHash> getHashEffAddCalculationFieldEncoding(OperationMod operationMod, const std::string &bitDisplacement);
std::unordered_map<std::bitset<8>, std::string, BitsetHash> getHashJumpEncoding();
std::unordered_map<std::bitset<3>, std::string, BitsetHash> getHashAddSubCmpTypeEncoding();

#endif //HW1_DECODINGHASHMAPS_H
