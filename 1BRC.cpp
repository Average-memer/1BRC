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
};

template<>
struct std::hash<char[10]> {
    //provide a custom hashing function for char[10]
    //if the char[] is longer than 10 items this will cause a buffer overflow
    std::size_t operator()(const char s[10]) const {
        std::size_t hash = 0;
        //combine hashes
        for (int i = 0; i < 10; ++i) {
            hash = hash * 31 + s[i];
        }
        return hash;
    }
};

//check if two char array are equal
struct CharArrayEqual {
    bool operator()(const char lhs[10], const char rhs[10]) const {
        return std::strncmp(lhs, rhs, 10) == 0;
    }
};

unsigned int calculateMean(const station& station) {
    return station.sum/station.count;
}

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

int16_t extractTemp(const std::string&  inputText, int semicolonIndex) {
    //extract the temperature by only looking at the very chars we actually need.
    //We don't need bounds checking because the structure of each line is consistent all the time.
    //TODO: find out performance implications of strcpy and ways of avoiding it.
    char* text = new char[inputText.length() + 1];
    std::strcpy(text, inputText.c_str()); //I am in danger
    int16_t output = 0;
    //determine sign
    if (inputText[semicolonIndex + 1] == 0x2D) { //0x2D == "-"
        output += (text[semicolonIndex + 2] - 48) * 100;
        output += (text[semicolonIndex + 3] - 48) * 10;
        output += text[semicolonIndex + 5] - 48;
        output *= -1; //set sign to negative
        //6 adds and 3 mults, times one billion at 4 GHZ is at least 2 seconds, assuming no parallelism or optimisations and single-cycle muliplications.
        //Just for this code path...
    }
    else
    {
        //this branch runs if the temp is positive, i.e. the char after the semicolon is not "-"
        //sign bit is already zero
        output += (text[semicolonIndex + 2] - 48) * 100;
        output += (text[semicolonIndex + 3] - 48) * 10;
        output += text[semicolonIndex + 5] - 48;
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
    return -1; //should never be reachable
}

int main()
{
    const std::vector<std::string> meas = load_data(R"(/Users/lionsteinheiser/Library/CloudStorage/OneDrive-Persönlich/Bilder/1 Dokumente/C++/Billion row challenge/measurements.txt)");
    std::unordered_map<char[10], station> stationRecord; //a record of all the stations to map into
    //the meat and potatoes loop
    for (const std::string & s : meas) {
        const int semi = findSemicolon(s);
        char temp = extractTemp(s, semi);
        std::cout << s << "Temperature: " << temp << std::endl;
    }

	return 0;
}
