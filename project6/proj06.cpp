#include <iostream>
#include <algorithm>
#include <vector>
#include <fstream>
#include <cstdint>
#include <string>
#include <array>

using std::string, std::array;
using std::cout, std::endl, std::string, std::vector;

// Registers:
// R0: 0000 R1: 0057 R2: 0000 R3: 0000
// R4: 0000 R5: 0000 R6: 0000 R7: 0000
// R8: 0000 R9: 0000 Ra: 00ff Rb: 0000
// Rc: 0000 Rd: 0000 Re: 0000 Rf: 0000
struct Register{
   uint16_t data = 0x0000;     
};
array<Register,16> registers;

// The data cache display will include appropriate column headers. For example:
// Cache Contents:
// V M Tag 0 1 2 3 4 5 6 7
// 0 0 0 000 00 00 00 00 00 00 00 00
// 1 0 0 000 00 00 00 00 00 00 00 00
struct CacheLine{
    int v = 0x0; /// < valid bit
    int m = 0x0; /// < modified bit
    int mag = 0x0; /// < tag
    uint8_t data[8] = {0}; //8 bytes initialized to 0

};
/// Cache is direct mapped ie CacheLine = Address mod CacheLineSize (8)
array<CacheLine,8> cache;

// RAM Contents (First 128 Bytes):
// 0000: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
// 0010: 00 00 00 00 00 00 00 00 00 00 00 00 70 00 00 00
struct RAM{
    std::vector<uint8_t> memory{128, 0};
};
/// RAM[0xABCD + offset] to get the data
///
void loadRam(std::ifstream &ramfile){
        

    string s((std::istreambuf_iterator<char>(ramfile)),
            std::istreambuf_iterator<char>());
    cout << s << endl;
    ramfile.close();
}

int main(int argc, char ** argv){

// Items in the line will be separated by exactly one space, and each line will terminate with a newline. For
// example:
// LDR 5 ebd8
// STR 4 0ac2
// LDR a 0ad0

    /**
     * Options are;
     * -input [FILE]
     * -debug
     * -ram [FILE]
     */
    string inputFilename;
    string ramFilename;
    bool debug = false;

    for (int i = 1; i < argc; i++) {
        string s(argv[i]);
        if (s == "-debug") debug = true;
        if (s == "-ram") ramFilename= argv[++i];
        if (s == "-input") inputFilename = argv[++i];
    }

    std::ifstream ifh(inputFilename);

    std::ifstream rfh(ramFilename);

    if( !rfh ){
        std::cerr << "Error with ram file" << endl;
        return 1;
    }else{
        loadRam(rfh);
    }

    if( !ifh ){
        std::cerr << "Error with input file" << endl;
        return 1;
    }




// 3. Your program may assume that the “-input” and the “-ram” files are formatted correctly:
// a) If the “-input” file can be accessed, the contents will be valid as per #1 under “Project Specifi-
// cations”.
// b) If the “-ram” file can be accessed, the contents will be valid as per #4 under “Project Specifi-
// cations”.
//
// 4. Your program must create the following data structures:
// a) A data structure representing the registers. All sixteen registers will be initialized to zero at the
// start of the simulation.
// b) A data structure representing the data cache. All of the entries will be initialized to zero at the
// start of the simulation.
// c) A data structure representing the RAM. All of the entries will be initialized to zero at the start
// of the simulation. If the user selects the “-ram” option, a subset of the RAM will be re-initialized
// using the contents of the specified file.
// 5. Assume that the microprocessor uses a “big endian” memory model: the most significant byte of
// a 2-byte item is stored in the lowest numbered memory address, and the least significant byte is
// stored at the highest numbered memory address.
// For example, if the value 0xabcd is copied into memory at address 0x0214, then the byte at address
// 0x0214 in RAM will contain the value 0xab and the byte at address 0x0215 in RAM will contain
// the value 0xcd.
// 6. Assume that the microprocessor uses an aligned memory model: the address of a 2-byte item will
// always be a multiple of 2 (and thus the least significant bit will be 0).
// 7. This project will be graded based on a similarity metric. Your goal should be to produce an output
// identical to the shared test cases (whitespace formatting does not matter).
// 8. You should still develop your own set of test cases which considers edge cases such as wrong input
// commands. However, you can assume an input file (if exists) to be formatted correctly.
    return 0;
}
