#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <queue>
#include <cctype>
#include <algorithm>

std::ofstream log("LOG.txt");


void errmsg(std::string msg){
    std::cout << "Error detected : " << msg << std::endl;
    exit(1);
}

void printVector(const std::vector<std::string> & v){
    long unsigned int i = 0;
    while (i < v.size()){
        std::cout << v[i] << std::endl;
        i++;
    }
}

void printStateChange(int cycle,int processNo, const std::string & originalState, const std::string newState){
    log << cycle << ": " <<"Process " << processNo << ": " << originalState << " -> " << newState << std::endl;
}

void printDebug(int cycle,int processNo, const std::string & instruction){
    log << cycle << ": " <<"Process " << processNo << ": " << instruction << std::endl;
}


int extractNum(const std::string& s) {
    int result = 0;

    for (char c : s) {
        if (std::isdigit(static_cast<unsigned char>(c))) {
            result = result * 10 + (c - '0');
        }
    }

    return result;
}

bool contains(const std::string& s, const std::string& sub) {
    return s.find(sub) != std::string::npos;
}

int main(int argc, char * argv[]){

    /// Make sure a valid command is actually supplied 
    bool showDebug = false;
    
    if (argc == 1){
        errmsg("invalid input");
    }
    int processNum = 0;
    try{
        processNum = std::stoi(argv[1]);
    }catch (const std::invalid_argument & e){
        errmsg("lets try inputting a number as an argument buddy");
    }

    if (argc == 3){
        try{
            std::string arg = argv[2];
            if (arg =="-debug"){
                showDebug = true;
            }
        }catch (const std::invalid_argument & e){
            errmsg("bad 3rd arg inputted");
        }
    }

    ///Read contents of processN for processNum amount of files
    ///Determine the priority of each process and then add them to a queue
    ///How do I want to store the contents of each file?
    ///Perhaps as 2D array processesStates = [process1 = [prio, instr1,instr2,...], process[2] = [prio, instr1,instr2,instr3]]...]

    std::vector<int> processInstructionStep(processNum,1);
    std::vector<std::vector<std::string>> instructions(processNum);
    for(int i = 0; i < processNum; i++){
        std::string filename = "process"+std::to_string(i+1);
        std::ifstream file(filename);
        std::string line;
        instructions[i].push_back(filename);
        while (std::getline(file,line)){
            instructions[i].push_back(line);
        }
    }

    /// Sort processes in descending order
    std::sort(instructions.begin(), instructions.end(),
          [](const std::vector<std::string>& a,
             const std::vector<std::string>& b)
          {
              int a0 = std::stoi(a[1]);
              int b0 = std::stoi(b[1]);
              return a0 > b0;   // descending
          });
    
    std::queue<std::vector<std::string>> readyQueue;
    std::vector<std::tuple<int,int,std::vector<std::string>>> blockedQueue;

    
    for(int i = 0; i < instructions.size(); i++){
        readyQueue.push(instructions[i]);
        //std::cout << "\nINSTRUCTIONS FOR " << std::endl;
        //printVector(instructions[i]);
    }
    int cycle = 0;
    int TIMER_INTERRUPT_INTERVAL = 5;
    std::vector<std::string> runningProcess = readyQueue.front();
    readyQueue.pop();
    int PC = 2;
    bool systemCallActivated = false;

    int runningProcessNo = extractNum(runningProcess[0]);
    printStateChange(cycle,runningProcessNo,"Ready" , "Running");
    int waitCycles = 0;
    int timeSlice = 0;



    /// Run until nothing in readyQueue and nothing in blockedQueue (waiting for something) and nothing Running

    ///blockedQueue
    /**
     * blockedQueue = [process = [time to sleep, instructions]...]
     */

    while(!runningProcess.empty() || !readyQueue.empty() || !blockedQueue.empty()){

        if (!runningProcess.empty())
            timeSlice++;
            
        if (runningProcess.empty()){
            if(!readyQueue.empty()){
                    runningProcess = readyQueue.front();
                    readyQueue.pop();
                    runningProcessNo = extractNum(runningProcess[0]);
                    printStateChange(cycle,runningProcessNo,"Ready" , "Running");
                    timeSlice = 1;
                    PC = processInstructionStep[runningProcessNo - 1]+1;

            }else{
                log << cycle << ": " << "CPU Idle" << std::endl;
            }
        }
        std::string instruction;
        if (!runningProcess.empty()){
            runningProcessNo = extractNum(runningProcess[0]);
            /// Restore PC of current process
             instruction = runningProcess[PC];

            /// Check for SYS_CAL
            if (contains(instruction, "SYS_CALL")){
                /// Handle SYS_CAL
                systemCallActivated = true;
                waitCycles = extractNum(instruction);
            }

            if (showDebug == true){
                printDebug(cycle,runningProcessNo,instruction);
                //log << timeSlice << std::endl;
                /// Print instruction @ each step
            }
        }

        if (timeSlice >= TIMER_INTERRUPT_INTERVAL || systemCallActivated){

            /// Save PC for this process
            processInstructionStep[runningProcessNo - 1] = PC;
            if (systemCallActivated){
                if (!contains(instruction,"TERMINATE")){
                    blockedQueue.emplace_back(cycle,waitCycles,runningProcess);
                    printStateChange(cycle,runningProcessNo,"Running" , "Blocked");
                }else{
                    printStateChange(cycle,runningProcessNo,"Running" , "Halted");
                }
                systemCallActivated = false;
            }else{
                readyQueue.push(runningProcess);
                printStateChange(cycle,runningProcessNo,"Running" , "Ready");

            }
            runningProcess.clear();
            if(!readyQueue.empty()){
                runningProcess = readyQueue.front();
                readyQueue.pop();
                runningProcessNo = extractNum(runningProcess[0]);
                printStateChange(cycle,runningProcessNo,"Ready" , "Running");
                timeSlice = 0;
                PC = processInstructionStep[runningProcessNo - 1];
            }
            /// Change current running process
        }

        for(int i = 0 ; i < blockedQueue.size(); i++){
            int stoppedOn = std::get<0>(blockedQueue[i]);
            int waitCycles = std::get<1>(blockedQueue[i]);
            if(cycle!=stoppedOn && (cycle-stoppedOn)>=waitCycles){
                readyQueue.push(std::get<2>(blockedQueue[i]));
                printStateChange(cycle,extractNum(std::get<2>(blockedQueue[i])[0]),"Blocked" , "Ready");
                blockedQueue.erase(blockedQueue.begin() + i);


            }

        }
        if (!runningProcess.empty())
            PC++;

        cycle++;





    }
    

    return 0;
}