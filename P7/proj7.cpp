#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <sstream>
#include <string>
#include <queue>

using namespace std;

const int RAM_LINE_BYTES = 16;
const int RAM_LINES = 8;
const int DISK_SIZE = 32;
const int PAGE_COUNT = 16;

typedef struct {
    int present;
    int modified;
    int access;
    int frame;
    int disk_area;
} page_line_t;

typedef struct {
    string name;
    int page_count;
    page_line_t page_table[PAGE_COUNT];
} process_t;

typedef struct {
    int data[RAM_LINE_BYTES];
} ram_line_t;

typedef struct {
    ram_line_t lines[RAM_LINES];
} ram_t;

ram_t RAM;
unsigned char Disk[DISK_SIZE * 16];

queue<int> fifo;
bool frame_used[RAM_LINES] = {false};
int next_disk_slot = 0;

unsigned short hexToShort(string s) {
    return (unsigned short) stoi(s, nullptr, 16);
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

void write_to_disk(int disk_area, int data[]) {
    for (int i = 0; i < 16; i++) {
        Disk[disk_area * 16 + i] = (unsigned char)data[i];
    }
}

void load_from_disk(int frame, int disk_area) {
    for (int i = 0; i < 16; i++) {
        RAM.lines[frame].data[i] = Disk[disk_area * 16 + i];
    }
}

int get_free_frame() {
    for (int i = 0; i < RAM_LINES; i++) {
        if (!frame_used[i]) {
            frame_used[i] = true;
            return i;
        }
    }
    return -1;
}

string evict_frame(vector<process_t>& processes, int &frame, string &wb) {

    frame = fifo.front();
    fifo.pop();

    for (auto &proc : processes) {
        for (int i = 0; i < PAGE_COUNT; i++) {
            auto &pte = proc.page_table[i];

            if (pte.present && pte.frame == frame) {

                if (pte.modified) {
                    write_to_disk(pte.disk_area, RAM.lines[frame].data);
                    wb = "y";
                } else {
                    wb = "n";
                }

                pte.present = 0;
                pte.frame = -1;

                return "F" + to_string(frame);
            }
        }
    }

    return "--";
}

short validate(process_t *p, string instr, int vpn, int offset,
               vector<process_t>& processes) {

    ofstream log("LOG.new",std::ios::app);

    auto &pline = p->page_table[vpn];

    string alloc = "--", ev = "--", wb = "--";
    short data = -1;

    if (!pline.access) {
        log << p->name << " " << instr << " " << vpn
             << " 0x" << hex << offset
             << " | PROT_FAULT | alloc=-- evict=-- wb=-- | PA=--\n";
        return -1;
    }

    if (!pline.present) {

        int frame = get_free_frame();

        if (frame == -1) {
            ev = evict_frame(processes, frame, wb);
        }

        alloc = "F" + to_string(frame);

        if (pline.disk_area == -1) {
            pline.disk_area = next_disk_slot++;
        } else {
            load_from_disk(frame, pline.disk_area);
        }

        pline.frame = frame;
        pline.present = 1;
        pline.modified = 0;

        fifo.push(frame);
    }

    int pa = pline.frame * 16 + offset;

    if (instr == "READ") {
        data = RAM.lines[pline.frame].data[offset];
    } else {
        RAM.lines[pline.frame].data[offset] = 1;
        pline.modified = 1;
    }

    string result = (alloc == "--") ? "hit" : "PAGE_FAULT";

    log << p->name << " " << instr << " " << vpn
         << " 0x" << hex << offset
         << " | " << result
         << " | alloc=" << alloc
         << " evict=" << ev
         << " wb=" << wb
         << " | PA=0x" << setw(2) << setfill('0') << pa
         << endl;

    log.close();
    return data;
}

int main(int argc, char* argv[]) {
    ofstream log("LOG.new");
    log.close();


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

    ifstream input(inputFile);
    if (!input) {
        cout << "file error\n";
        return 1;
    }

    int process_count;
    input >> process_count;

    vector<process_t> processes;

    for (int i = 0; i < process_count; i++) {
        process_t p{};
        input >> p.name >> p.page_count;

        for (int j = 0; j < PAGE_COUNT; j++) {
            p.page_table[j] = {0,0,1,-1,-1};
        }

        processes.push_back(p);
    }

     if (debug) {
         printRAM();
         printDisk();
         for (auto p : processes)
             print_page_table(&p);
     }

    string pname, instr, vaddr;

    while (input >> pname >> instr >> vaddr) {

        int addr = hexToShort(vaddr);
        int vpn = (addr >> 4) & 0xF;
        int offset = addr & 0xF;

        for (auto &p : processes) {
            if (p.name == pname) {
                validate(&p, instr, vpn, offset, processes);
            }
        }

        if(debug){
         for (auto p : processes)
             print_page_table(&p);
        }
    }
    printRAM();
    printDisk();


    return 0;
}

