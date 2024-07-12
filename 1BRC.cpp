#include "1BRC.h"

#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <unordered_map>
//based off of https://github.com/gunnarmorling/1brc/tree/main

struct station
{
    char identifier[11]; //the first ten characters ought to be enough to identify each unique station. 11 due to the null terminator
    unsigned int sum; //sum of all measurement entries for that station
    int count; //how often that station appears in the data, used for calculating the mean
    int min; //minimum temp
    int max; //maximum

    station(const char* id, unsigned int sum = 0, int count = 0, int min = 0, int max = 0)
        : sum(sum), count(count), min(min), max(max)
    {
        // Copy the identifier ensuring it does not overflow and is null-terminated
        strncpy(identifier, id, 10);
        identifier[10] = '\0'; // Ensure null-termination
    }
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
    //we can factor out the times ten from all our samples and cancel it with a 10 in the denominator
    //This allows us to only do integer arithmetic in the main loop.
    return station.sum/(station.count * 10);
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

int16_t extractTemp(const std::string&  inputText) {
    //find where the semicolon lives to use as an index later.
    size_t semicolonIndex = 0;
    char* text = new char[inputText.length() + 1];
    memcpy(text, inputText.c_str(), inputText.size() + 1); //memcpy is faster than strcpy
    int i = 0;
    while (text[i] != 0x00 && i < inputText.length() + 1) //iterate over each char
    {
        if (text[i] == 0x3b) //check for semicolon
        {
            semicolonIndex = i;
            break;
        }
        ++i;
    }
    //handle short length names:
    //The hash function expects ten char long identifiers, so we pad it to that length.
    if (semicolonIndex < 10) {
    char key[10]; // use this to update the station map
        //copy contents of existing name
        for (int j = 0; j < 10; j++) {
            if (j < semicolonIndex) {
                key[j] = text[j];
            }
            else {
                key[j] = ' ';
            }
        }
    }


    //extract the temperature by only looking at the very chars we actually need.
    //We don't need bounds checking because the structure of each line is consistent all the time.
    int16_t output = 0;
    //determine sign
    if (inputText[semicolonIndex + 1] == 0x2D) { //0x2D == "-"
        output += (text[semicolonIndex + 2] - 48) * 100;
        output += (text[semicolonIndex + 3] - 48) * 10;
        output += text[semicolonIndex + 5] - 48;
        output *= -1; //set sign to negative
        //6 adds and 2 mults, times one billion at 4 GHZ is at least 2 seconds, assuming no parallelism or optimisations and single-cycle muliplications.
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
    //update map
    //TODO add entries to map in an efficient way. Maybe add the stations we have already added to a seperate list by hash and only insert ones we havent yet added? That avoids std::try_emplace, which might be slow.

    return output;
}

//global ?
std::unordered_map<char[10], station> stationRecord; //a record of all the stations to map into

int main(){
    const std::vector<std::string> meas = load_data(R"(/Users/lionsteinheiser/Library/CloudStorage/OneDrive-Persönlich/Bilder/1 Dokumente/C++/Billion row challenge/measurements.txt)");
    //the meat and potatoes loop
    for (const std::string & s : meas) {
        int16_t temp = extractTemp(s);
        std::cout << s << "Temperature: " << temp << std::endl;
    }

	return 0;
}
