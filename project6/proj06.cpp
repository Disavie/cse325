#include <iostream>
#include <algorithm>
#include <vector>
#include <fstream>
 
using std::cout, std::endl, std::string, std::vector;
// We will have 16 Registers with 4 Hex eachc
// Registers:
// R0: 0000 R1: 0057 R2: 0000 R3: 0000
// R4: 0000 R5: 0000 R6: 0000 R7: 0000
// R8: 0000 R9: 0000 Ra: 00ff Rb: 0000
// Rc: 0000 Rd: 0000 Re: 0000 Rf: 0000
struct Register{
    
};

// The data cache display will include appropriate column headers. For example:
// Cache Contents:
// V M Tag 0 1 2 3 4 5 6 7
// 0 0 0 000 00 00 00 00 00 00 00 00
// 1 0 0 000 00 00 00 00 00 00 00 00
// 2 0 0 000 00 00 00 00 00 00 00 00
// 3 1 0 000 00 00 00 00 70 00 00 00
// 4 1 0 2ac 00 00 00 00 00 00 00 00
// 5 0 0 000 00 00 00 00 00 00 00 00
// 6 1 0 001 00 00 00 00 00 00 00 00
// 7 0 0 000 00 00 00 00 00 00 00 00
struct Cache{

};

// RAM Contents (First 128 Bytes):
// 0000: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
// 0010: 00 00 00 00 00 00 00 00 00 00 00 00 70 00 00 00
// 0020: 00 00 00 00 6d 00 00 00 00 00 00 00 00 00 00 00
// 0030: 00 ff 00 00 00 00 00 00 00 00 00 00 00 00 00 23
// 0040: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
// 0050: 7b 00 23 cc 89 0a 00 ef 00 ff 00 00 00 00 00 00
// 0060: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
// 0070: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
struct Ram{

};

int main(int argc, char ** argv){

// 1. The program will simulate the interactions between the register unit, the data cache, and primary
// storage by processing a file which contains zero or more instructions. Each line of the file will contain
// the following information:
// a) Operation (“LDR” for “load into register” or “STR” for “store from register”)
// b) Register number (one hexadecimal digit)
// c) Physical address (four hexadecimal digits, with leading zeroes)
// Items in the line will be separated by exactly one space, and each line will terminate with a newline. For
// example:
// LDR 5 ebd8
// STR 4 0ac2
// LDR a 0ad0

   cout << "Hello world" << endl;

    return 0;
}
