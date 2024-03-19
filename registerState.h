//
// Created by rob on 18/03/24.
//

#ifndef HW1_REGISTERSTATE_H
#define HW1_REGISTERSTATE_H

#include <iostream>
#include <unordered_map>
#include <string>

std::unordered_map<std::string, int> initializeRegisterValueMap();
void printRegisterValueMap(const std::unordered_map<std::string, int> &registerValueMap);
bool updateRegisterValueMapAndGetSignFlag(std::unordered_map<std::string, int> &registerValueMap, const std::string &key, int newValue);
void updateRegisterValueMap(std::unordered_map<std::string, int> &registerValueMap, const std::string &key, int newValue);

#endif //HW1_REGISTERSTATE_H
