// proj07 - extended from proj06
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <cstring>
#include <string>
#include <stdio.h>

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

void loadDisk(string filename) {
    ifstream file(filename);
    string line;

    //skip first 2 lines
    std::getline(file,line);
    std::getline(file,line);
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

void printDisk() {

    cout << "\n Disk: \n";


    for (int i = 0; i < 16*32; i += 16) {
        cout << hex << i << ": ";
        //cout << setw(2) << setfill('0') << hex << i << ": ";

        for (int j = 0; j < 16; j++) {
            cout << setw(2) << setfill('0')
                 << (int)Disk[i/16 + j];

            if (j < 15) cout << " ";
        }
        cout << "\n";
    }
}


void populate_page_table(process_t * p){
    string pname = p->name;
    auto page_table = p->page_table;

    string filename = "pageTable_" + pname + ".old";
    ifstream file(filename);
    string line;

    //skip first 1 line
    std::getline(file,line);
    int i = 0; 
    while (std::getline(file,line)){
        stringstream ss(line);
        page_line_t page_line = {};
        ss >> page_line.present;
        ss >> page_line.modified;
        ss >> page_line.access;
        string check;
        ss >> check;
        if (check == "-"){
            page_line.frame = -1;
        }else{
            page_line.frame = std::stoi(check);
        }
        ss >> check;
        if (check == "-"){
            page_line.disk_area = -1;
        }else{
            page_line.disk_area = std::stoi(check);
        }

       page_table[i++] = page_line;
    }
}

void print_page_table(process_t * p){
    auto page_table = p->page_table;

    for (int i = 0 ; i < PAGE_COUNT; i++){
        cout << page_table[i].present << ' ';
        cout << page_table[i].modified << ' ';
        cout << page_table[i].access << ' ';
        int check;
        check = page_table[i].frame;
        if (check == -1){
            cout << '-' << ' ';
        }else{
            cout << check << ' ';
        }

        check = page_table[i].disk_area;
        if (check == -1){
            cout << '-' << ' ';
        }else{
            cout << check << ' ';
        }
        cout << '\n';
    }
}

short unsigned validate(process_t * p, string instr, short addr, int offset){

    short unsigned d = -1;
    // search in page table
    page_line_t pline = (p->page_table)[addr];
    /*
    cout << "addr = " << addr << 
        "\npline : present = " << pline.present <<
        "\npline : frame = " << pline.frame << endl;
        */

    if (pline.present ){
        short pa = (pline.frame << 4) | offset;
        cout << "PA = 0x" << hex << pa << endl;
    }else{
        int disk_area = pline.disk_area;
        //retrieve from disk
        cout << "Need to retrieve from disk for " << instr << " at " << addr << offset << endl;
    }

    return d;
}


int main(int argc, char* argv[]) {

        cout << hex << nouppercase;
    string inputFile, ramFile, diskFile;

    bool debug = false;

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-input") inputFile = argv[++i];
        else if (arg == "-ram") ramFile = argv[++i];
        else if (arg == "-debug") debug = true;
        else if (arg == "-disk") diskFile = argv[++i];
    }

    initSystem();
    //todo is add process page table init

    if (!ramFile.empty()) loadRAM(ramFile);
    if (!diskFile.empty()) loadDisk(diskFile);

    


    std::ifstream input(inputFile);
    if (!input) {
        std::cout << "uh oh\n";
        exit(1);
    }

    int process_count;
    input >> process_count;

    std::vector<process_t> processes;
    for (int i = 0; i < process_count; i++) {
        process_t p{};
        input >> p.name >> p.page_count;
        processes.push_back(p);
    }

    for(int i = 0 ; i < process_count ; i++ ){
        auto *p = &processes[i];
        populate_page_table(p);
    }

     if (debug) {
         printRegisters();
         printRAM();
         printDisk();
         //printCache();
         for (auto p : processes)
             print_page_table(&p);
     }
     //exit(1);

    string pname, instr, vaddr;
    
    //read from stdin, operation, register address seperated by whitespace
    while (input >> pname >> instr >> vaddr) {
        //vaddr ->  real addr 4bit VPN + 4bit offset
        
        //int regNum = hexToShort(reg);
        unsigned short address = hexToShort(vaddr);
        int vpn = (address >> 4) & 0xF;
        int offset = address & 0x0F;

        cout << vpn << ' ' << offset << endl;


//        int tag = (address >> 6) & 0x3FF;

        bool hit;
        unsigned short data;

        for(int i = 0 ; i < process_count ; i++ ){
            auto *p = &processes[i];
            if (p->name == pname){
                //print_page_table(p);
                data = validate(p,instr,vpn,offset);
            }
        }

        if (data == -1){
            exit(1);
        }
        continue;

        /*
        cout << op << " "
            << hex << nouppercase
            << reg << " "
            << setw(4) << setfill('0') << vaddr << " "
            << setw(3) << setfill('0') << tag << " "
            << index << " "
            << offset << " "
            << (hit ? "H" : "M") << " "
            << setw(4) << setfill('0') << data
            << "\n";
            */

        if (debug) {
            printCache();
        }
    }

    printRegisters();
    printCache();
    printRAM();


    return 0;
}
