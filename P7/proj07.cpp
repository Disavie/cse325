// proj07 - extended from proj06
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <cstring>
#include <string>
#include <stdio.h>
#include <queue>

using namespace std;

const int BLOCK_SIZE = 8;

const int RAM_LINE_BYTES = 16;
const int RAM_LINES = 8;

// new for proj7
const int DISK_SIZE = 32;
const int PAGE_COUNT = 16;



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

typedef struct{
    int data[RAM_LINE_BYTES];
}ram_line_t;

typedef struct{
   ram_line_t lines[RAM_LINES]; 
}ram_t;

ram_t RAM;

unsigned char Disk[DISK_SIZE];

unsigned short hexToShort(string s) {
    return (unsigned short) stoi(s, nullptr, 16);
}

string toHex(int val, int width) {
    stringstream ss;
    ss << hex << setw(width) << setfill('0') << nouppercase << val;
    return ss.str();
}

void loadRAM(string filename) {
    ifstream file(filename);
    string frame;

    //skip first 2 lines
    std::getline(file,frame);
    std::getline(file,frame);

    while (file >> frame) {
        int base = hexToShort(frame);
        for (int i = 0; i < 16; i++) {
            string byte;
            file >> byte;

            RAM.lines[base].data[i] = (unsigned char) hexToShort(byte);
        }
    }
}


void printRAM() {
    cout << "\n";

    for (int i = 0; i < RAM_LINES; i++) {
        cout << setw(4) << setfill('0') << hex << nouppercase << i << ": ";

        for (int j = 0; j < 16; j++) {
            cout << setw(2) << setfill('0')
                 << (int)RAM.lines[i].data[j];

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

inline void write_to_disk(int base, int offset, int data[]){
    Disk[base+offset] = (unsigned char)data[offset];
}

short unsigned validate(process_t * p, string instr, short addr, int offset){

    short unsigned d = -1;
    // search in page table
    page_line_t pline = (p->page_table)[addr];
    string log = p->name + " " + instr + " " + to_string(addr);
    string pa = toHex((pline.frame << 4) | offset,2);

    log+= " 0x" + pa + " | ";

    if (!pline.access){
        log += "PROT_FAULT | alloc=-- evict=-- wb=-- | PA=--";
    }else{
        if (!pline.present){

            if (pline.modified){
                write_to_disk(pline.disk_area,offset, RAM.lines[pline.frame].data);
            }
            //evict()
            //load()
            
            if (instr == "READ"){
                //d = RAM.lines[pline.frame].data[offset];
                pline.modified = 0;
            else{ 
                //write_to_ram()
                pline.modified = 1;
            }

            //pline.disk_area = ;
            //pline.frame = ;
            pline.present = 1;
            // UNFINISHED >>
            log+= "PAGE_FAULT | alloc=-- evict=-- wb=-- | PA=" + pa; 
        }else{

            if (instr == "READ"){
                d = RAM.lines[pline.frame].data[offset];
            }else{
                write_to_disk(pline.disk_area,offset, RAM.lines[pline.frame].data);
                pline.modified = 1;
            }
            log+= "hit | alloc=-- evict=-- wb=-- | PA="+pa;
        }
    }
    cout << log << endl;
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
         printRAM();
         printDisk();
         for (auto p : processes)
             print_page_table(&p);
     }

    string pname, instr, vaddr;
    
    //read from stdin, operation, register address seperated by whitespace
    while (input >> pname >> instr >> vaddr) {
        //vaddr ->  real addr 4bit VPN + 4bit offset
        
        //int regNum = hexToShort(reg);
        unsigned short address = hexToShort(vaddr);
        int vpn = (address >> 4) & 0xF;
        int offset = address & 0x0F;


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

        if (data != -1){
            continue;
        }else{
            //exit(1);
        }

    }

    printRAM();
    printDisk();


    return 0;
}
