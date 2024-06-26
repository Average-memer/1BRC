#include "1BRC.h"

#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
//based off of https://github.com/gunnarmorling/1brc/tree/main

struct station
{
    std::string name;
    int sum; //sum of all measurement entries for that station
    int count; //how often that station appears in the data, used for calculating the mean
    int min; //minimum temp
    int max; //maximum temp
    int calculateMean() const {
        return sum / count;
    }
};

std::vector<std::string> load_data(const std::string& path) {
    //loads the lines of a file into a vector of strings, one string per line
    std::ifstream file(path);
    std::vector<std::string > lines;
    std::string placeholder;
    while (std::getline(file, placeholder)) {
        lines.emplace_back(placeholder);
    }
    return lines;
}

void printBytes(const std::string& input) {
    //print a string as its hex representation
    for (unsigned char c : input) {
        std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c) << " ";
    }
    std::cout << std::endl;
}

int findSemicolon(const std::string& input) {
    //it works!
    //turn string into char array
    char* carray = new char[input.length() + 1];
    std::strcpy(carray, input.c_str()); //I am in danger

    int i = 0;
    while (carray[i] != 0x00) //iterate over each char
    {
        if (carray[i] == 0x3b) //check for semicolon
        {
            return i;
        }
        ++i;
    }
}

int main()
{
    auto meas = load_data(R"(C:\Users\lions\OneDrive\Bilder\1 Dokumente\C++\Billion row challenge\measurements.txt)");
    for (std::string s : meas) {
        std::cout << s << ": " << findSemicolon(s) << std::endl;
    }

	return 0;
}
