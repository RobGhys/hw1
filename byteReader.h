//
// Created by rob on 18/03/24.
//

#ifndef HW1_BYTEREADER_H
#define HW1_BYTEREADER_H
#include <iostream>
#include <bitset>
#include <vector>
#include "instructionDecoding.h"

std::string readExtraBytes(std::ifstream &inputFile, int bytesToRead);
std::string readDataBytes(std::ifstream &inputFile);
std::bitset<8> readExtraByte(std::ifstream &inputFile);
int convertOneByteBase2ToBase10(const std::bitset<8> &secondByte);
int convertTwoByteBases2ToBase10(const std::bitset<16> &bytes);
std::vector<std::bitset<8>> getJumpInstructionBytes();
bool checkSixBitsInRegister(const TwoBytes &inputBits, const std::bitset<6> &instructionBits);
bool checkSevenBitsInRegister(const TwoBytes &inputBits, const std::bitset<7> &instructionBits);
TwoBytes &getSixteenBits(bool littleEndian, uint16_t twoBytes, std::bitset<16> &binaryTwoBytes, TwoBytes &sixteenBits);
void readExtraByteAndDoNothing(std::ifstream &inputFile);

#endif //HW1_BYTEREADER_H
