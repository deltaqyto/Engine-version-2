#pragma once
#ifndef TEXT_H
#define TEXT_H
#include <string>
#include <vector>
#include <sstream>

std::vector < std::string> split(std::string input, char character);

std::vector <std::vector<std::string>> split(std::vector<std::string> input, char character);

#endif