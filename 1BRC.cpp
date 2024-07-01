#include "1BRC.h"

#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <unordered_map>
//based off of https://github.com/gunnarmorling/1brc/tree/main

struct station
{
    char identifier[10]; //the first ten characters ought to be enough to identify each unique station.
    unsigned int sum; //sum of all measurement entries for that station
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

char extractTemp(const char* inputText, int semicolonIndex) {
    //extract the temperature by only looking at the very chars we actually need.
    //We don't need bounds checking because the structure of each line is consistent all the time.
    char output = 0;
    //determine sign
    if (inputText[semicolonIndex + 1] == 0x2D) { //0x2D == "-"
        output |= (1 << 8); //toggle the sign bit to negative
        output += inputText[semicolonIndex + 2] * 10;
        output += inputText[semicolonIndex + 3] * 10;
        output += inputText[semicolonIndex + 5] * 10;
        //6 adds and 3 mults, times one billion at 4 GHZ is at least 2 seconds, assuming no parallelism or optimisations and single-cycle muliplications.
        //Just for this code path...
    }
    else
    {
        //this branch runs if the temp is positive, i.e. the char after the semicolon is not "-"
        //sign bit is already zero
        output += inputText[semicolonIndex + 1] * 10;
        output += inputText[semicolonIndex + 2] * 10;
        output += inputText[semicolonIndex + 4] * 10;
    }
    return output;
}

int findSemicolon(const std::string& input) {
    //it works!
    //turn string into char array
    char* carray = new char[input.length() + 1];
    std::strcpy(carray, input.c_str()); //I am in danger

    int i = 0;
    while (carray[i] != 0x00 && i < input.length() + 1) //iterate over each char
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
    std::unordered_map<char[10], station> stationRecord; //a record of all the stations to map into
    //the meat and potatoes loop
    for (const std::string s : meas) {
        const int semi = findSemicolon(s);
        char temp = extractTemp(s, semi);
        std::cout << s << ": " << findSemicolon(s) << std::endl;
    }

	return 0;
}
