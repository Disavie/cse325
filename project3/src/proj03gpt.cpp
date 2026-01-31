#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <algorithm>
#include <string>

using namespace std;

struct Process {
    int pid;
    int priority;
    int pc = 0;
    vector<string> instructions;
    int blockRemaining = 0;
};

bool isSysCall(const string& s) {
    return s.find("SYS_CALL") != string::npos;
}

int extractNum(const string& s) {
    int n = 0;
    for (char c : s)
        if (isdigit(c)) n = n * 10 + (c - '0');
    return n;
}

int main(int argc, char* argv[]) {
    if (argc < 2) return 1;

    int N = stoi(argv[1]);
    bool debug = (argc == 3 && string(argv[2]) == "-debug");

    ofstream log("LOG.txt");

    vector<Process> processes;

    for (int i = 1; i <= N; i++) {
        ifstream file("process" + to_string(i));
        if (!file) continue;

        Process p;
        p.pid = i;

        file >> p.priority;
        file.ignore();

        string line;
        while (getline(file, line))
            p.instructions.push_back(line);

        processes.push_back(p);
    }

    sort(processes.begin(), processes.end(),
         [](const Process& a, const Process& b) {
             return a.priority > b.priority;
         });

    queue<Process*> ready;
    vector<Process*> blocked;
    Process* running = nullptr;

    for (auto& p : processes)
        ready.push(&p);

    int cycle = 0;
    int timer = 0;
    const int TIMER = 5;

    auto dispatch = [&]() {
        if (!ready.empty()) {
            running = ready.front();
            ready.pop();
            timer = 0;
            log << cycle << ": Process " << running->pid
                << ": Ready -> Running\n";
        }
    };

    dispatch();

    while (running || !ready.empty() || !blocked.empty()) {
        // unblock processes
        for (int i = 0; i < blocked.size(); ) {
            blocked[i]->blockRemaining--;
            if (blocked[i]->blockRemaining == 0) {
                log << cycle << ": Process " << blocked[i]->pid
                    << ": Blocked -> Ready\n";
                ready.push(blocked[i]);
                blocked.erase(blocked.begin() + i);
            } else {
                i++;
            }
        }

        if (!running) {
            dispatch();
            cycle++;
            continue;
        }

        if (running->pc >= running->instructions.size()) {
            log << cycle << ": Process " << running->pid
                << ": Running -> Halted\n";
            running = nullptr;
            dispatch();
            cycle++;
            continue;
        }

        string instr = running->instructions[running->pc];

        if (debug)
            log << cycle << ": Process " << running->pid
                << ": " << instr << "\n";

        running->pc++;
        timer++;

        if (isSysCall(instr)) {
            if (instr.find("TERMINATE") != string::npos) {
                log << cycle << ": Process " << running->pid
                    << ": Running -> Halted\n";
                running = nullptr;
                dispatch();
            } else {
                running->blockRemaining = extractNum(instr);
                log << cycle << ": Process " << running->pid
                    << ": Running -> Blocked\n";
                blocked.push_back(running);
                running = nullptr;
                dispatch();
            }
        } else if (timer == TIMER) {
            log << cycle << ": Process " << running->pid
                << ": Running -> Ready\n";
            ready.push(running);
            running = nullptr;
            dispatch();
        }

        cycle++;
    }

    log.close();
    return 0;
}
