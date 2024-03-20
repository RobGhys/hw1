//
// Created by rob on 18/03/24.
//

#include <iomanip>
#include "registerState.h"


std::unordered_map<std::string, int> initializeRegisterValueMap() {
    std::unordered_map<std::string, int> result{};

    result["ax"] = 0x0;
    result["bx"] = 0x0;
    result["cx"] = 0x0;
    result["dx"] = 0x0;
    result["sp"] = 0x0;
    result["bp"] = 0x0;
    result["si"] = 0x0;
    result["di"] = 0x0;

    return result;
}

void printRegisterValueMap(const std::unordered_map<std::string, int> &registerValueMap) {
    for (const auto &pair : registerValueMap) {
        // Left-align the register name and set a minimum width
        std::cout << std::left << std::setw(6) << pair.first << ": 0x"
                  << std::hex << std::setfill('0') << std::setw(4) << pair.second
                  // Reset fill character for decimal output
                  << std::setfill(' ') << " (" << std::dec << pair.second << ")" << std::endl;
    }
}

bool updateRegisterValueMapAndGetSignFlag(std::unordered_map<std::string, int> &registerValueMap, const std::string &key, int newValue) {
    auto iterator = registerValueMap.find(key);
    if (iterator != registerValueMap.end()) {
        iterator->second = newValue;

        // MSB is 15th bit. Shift 15 times to the right. Use logical AND.
        bool signFlag = (newValue >> 15) & 1;
        /*if (signFlag) {
            std::cout << "S -> 1"  << "val: " << newValue << std::endl;
        } else {
            std::cout << "S -> 0" << std::endl;

        }*/
        return signFlag;
    }
    return false;
}

void updateRegisterValueMap(std::unordered_map<std::string, int> &registerValueMap, const std::string &key, int newValue) {
    auto iterator = registerValueMap.find(key);
    if (iterator != registerValueMap.end()) {
        iterator->second = newValue;
    }
}
