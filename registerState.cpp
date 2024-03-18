//
// Created by rob on 18/03/24.
//

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
        std::cout << pair.first << " -> " << std::hex << pair.second << std::endl;
    }
}

void updateRegisterValueMap(std::unordered_map<std::string, int> &registerValueMap, const std::string &key, int newValue) {
    auto iterator = registerValueMap.find(key);
    if (iterator != registerValueMap.end()) {
        iterator->second = newValue;
    }
}