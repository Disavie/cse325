// proj07 - extended from proj06
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <cstring>
#include <string>
//thank you chat gpt for formatting the printing so there is minimal diffing between my stdout and expected stdout:w

using namespace std;

const int RAM_SIZE = 65536;
const int NUM_REGS = 16;
const int CACHE_LINES = 8;
const int BLOCK_SIZE = 8;

// new for proj7
const int DISK_SIZE = 32;
const int PAGE_COUNT = 16;

struct CacheLine {
    bool valid;
    bool modified;
    unsigned short tag;
    unsigned char data[BLOCK_SIZE];
};

unsigned short REG[NUM_REGS];
unsigned char RAM[RAM_SIZE];
unsigned char Disk[DISK_SIZE];
CacheLine CACHE[CACHE_LINES];

unsigned short hexToShort(string s) {
    return (unsigned short) stoi(s, nullptr, 16);
}

string toHex(int val, int width) {
    stringstream ss;
    ss << hex << setw(width) << setfill('0') << nouppercase << val;
    return ss.str();
}

void initSystem() {
    memset(REG, 0, sizeof(REG));
    memset(RAM, 0, sizeof(RAM));

    for (int i = 0; i < CACHE_LINES; i++) {
        CACHE[i].valid = 0;
        CACHE[i].modified = 0;
        CACHE[i].tag = 0;
        memset(CACHE[i].data, 0, BLOCK_SIZE);
    }
}

void loadDisk(string filename) {
    ifstream file(filename);
    string addr;
    while (file >> addr) {
        int base = hexToShort(addr);
        for (int i = 0; i < 16; i++) {
            string byte;
            file >> byte;
            Disk[base + i] = (unsigned char) hexToShort(byte);
        }
    }
}

void loadRAM(string filename) {
    ifstream file(filename);
    string addr;
    while (file >> addr) {
        int base = hexToShort(addr);
        for (int i = 0; i < 16; i++) {
            string byte;
            file >> byte;
            RAM[base + i] = (unsigned char) hexToShort(byte);
        }
    }
}

void writeBack(int index) {
    if (CACHE[index].valid && CACHE[index].modified) {
        unsigned short tag = CACHE[index].tag;
        int baseAddr = (tag << 6) | (index << 3);

        for (int i = 0; i < BLOCK_SIZE; i++) {
            RAM[baseAddr + i] = CACHE[index].data[i];
        }
    }
}

void loadBlock(int index, unsigned short tag, unsigned short addr) {
    writeBack(index);

    int baseAddr = addr & 0xFFF8; // align to 8-byte block

    for (int i = 0; i < BLOCK_SIZE; i++) {
        CACHE[index].data[i] = RAM[baseAddr + i];
    }

    CACHE[index].tag = tag;
    CACHE[index].valid = 1;
    CACHE[index].modified = 0;
}

unsigned short accessCache(string op, int regNum, unsigned short addr, bool &hit) {

    int offset = addr & 0x7;
    int index = (addr >> 3) & 0x7;
    int tag = (addr >> 6) & 0x3FF;

    CacheLine &line = CACHE[index];

    if (line.valid && line.tag == tag) {
        hit = true;
    } else {
        hit = false;
        loadBlock(index, tag, addr);
    }

    // BIG ENDIAN
    if (op == "LDR") {
        unsigned short value =
            (line.data[offset] << 8) |
            (line.data[offset + 1]);

        REG[regNum] = value;
        return value;
    } else { // STR
        unsigned short value = REG[regNum];

        line.data[offset]     = (value >> 8) & 0xFF;
        line.data[offset + 1] = value & 0xFF;

        line.modified = 1;
        return value;
    }
}

// ---------------- PRINT FUNCTIONS ----------------
void printRegisters() {
    cout << "\n";

    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            int i = row + col * 4;

            cout << "R" << hex << nouppercase << i << ": "
                 << setw(4) << setfill('0') << REG[i];

            if (col < 3) cout << "\t";
        }
        cout << "\n";
    }
}

void printCache() {
    cout << "\n";
    cout << "     V M Tag  0  1  2  3  4  5  6  7\n";
    cout << "------------------------------------\n";

    for (int i = 0; i < CACHE_LINES; i++) {
        cout << "[" << hex << i << "]: "
             << CACHE[i].valid << " "
             << CACHE[i].modified << " "
             << setw(3) << setfill('0') << CACHE[i].tag << " ";

        for (int j = 0; j < BLOCK_SIZE; j++) {
            cout << setw(2) << setfill('0')
                 << hex << (int)CACHE[i].data[j];

            if (j < BLOCK_SIZE - 1) cout << " ";
        }
        cout << "\n";
    }
}

void printRAM() {
    cout << "\n";

    for (int i = 0; i < 128; i += 16) {
        cout << setw(4) << setfill('0') << hex << nouppercase << i << ": ";

        for (int j = 0; j < 16; j++) {
            cout << setw(2) << setfill('0')
                 << (int)RAM[i + j];

            if (j < 15) cout << " ";
        }
        cout << "\n";
    }
}

typedef struct {
    int present;
    int modified;
    int access;
    int frame;
    int disk_area;

}page_line_t;

typedef struct {
    std::string name;
    int page_count;
    page_line_t page_table[PAGE_COUNT];

}process_t;

int main(int argc, char* argv[]) {

        cout << hex << nouppercase;
    string inputFile, ramFile, diskFile;

    bool debug = false;

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-input") inputFile = argv[++i];
        else if (arg == "-ram") ramFile = argv[++i];
        else if (arg == "-debug") debug = true;
        else if (arg == "--disk") diskFile = argv[++i];
    }

    initSystem();

    //if (!ramFile.empty()) loadRAM(ramFile);
    //if (!diskFile.empty()) loadDisk(diskFile);

    // if (debug) {
    //     printRegisters();
    //     printCache();
    //     printRAM();
    // }
    
    ifstream input(inputFile);

    string line;
    getline(input, line);
    stringstream ss(line);
    int process_count;
    ss >> process_count;

    std::vector<process_t> processes;
    for (int i = 0 ; i < process_count; i++){
        getline(input, line);
        cout << line << endl;;
        stringstream ss(line);
        process_t p = {};
        int x;
        ss >> p.name >> x;
        cout << p.name << '\t' << p.page_count << endl;
        processes.push_back(p);
    }

    

    string pname, instr, vaddr;
    
    //read from stdin, operation, register address seperated by whitespace
    while (input >> pname >> instr >> vaddr) {
        /*
        int regNum = hexToShort(reg);
        unsigned short address = hexToShort(addr);

        int offset = address & 0x7;
        int index = (address >> 3) & 0x7;
        int tag = (address >> 6) & 0x3FF;

        bool hit;
        unsigned short data = accessCache(op, regNum, address, hit);

        cout << op << " "
            << hex << nouppercase
            << reg << " "
            << setw(4) << setfill('0') << addr << " "
            << setw(3) << setfill('0') << tag << " "
            << index << " "
            << offset << " "
            << (hit ? "H" : "M") << " "
            << setw(4) << setfill('0') << data
            << "\n";

        if (debug) {
            printCache();
        }
        */
    }

    printRegisters();
    printCache();
    printRAM();

    for(auto p : processes){
        cout << p.name << '\t' << p.page_count << endl;
    }

    return 0;
}
