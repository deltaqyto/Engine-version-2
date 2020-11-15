#include "text.h"

std::vector <std::string> split(std::string input, char character) {
    std::stringstream ss(input);
    std::string item;
    std::vector<std::string> splittedStrings;
    while (std::getline(ss, item, character)) {
        splittedStrings.push_back(item);
    }
    return splittedStrings;
}

std::vector <std::vector<std::string>> split(std::vector<std::string> input, char character) {
    std::vector <std::vector<std::string>> out;
    for (size_t i = 0; i < input.size(); i++)
    {
        std::stringstream ss(input[i]);
        std::string item;
        std::vector<std::string> splittedStrings;
        while (std::getline(ss, item, character)) {
            splittedStrings.push_back(item);
        }
        out.push_back(splittedStrings);
    }
    return out;
}