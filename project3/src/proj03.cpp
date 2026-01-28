#include <iostream>
#include <string>
#include <fstream>
#include <vector>

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

int main(int argc, char * argv[]){

    /// Make sure a valid command is actually supplied 
    std::cout << argc << std::endl;
    
    if (argc == 1){
        errmsg("invalid input");
    }
    int processNum = 0;
    try{
        processNum = std::stoi(argv[1]);
    }catch (const std::invalid_argument & e){
        errmsg("lets try inputting a number as an argument buddy");
    }

    ///Read contents of processN for processNum amount of files
    ///Determine the priority of each process and then add them to a queue
    ///How do I want to store the contents of each file?
    ///Perhaps as 2D array processesStates = [process1 = [prio, instr1,instr2,...], process[2] = [prio, instr1,instr2,instr3]]...]

    std::vector<int> processInstructionStep(processNum,0);
    std::vector<std::vector<std::string>> instructions(processNum);
    for(int i = 0; i < processNum; i++){
        std::string filename = "process"+std::to_string(i+1);
        std::ifstream file(filename);
        std::string line;
        while (std::getline(file,line)){
            instructions[i].push_back(line);
        }
    }
    for(int i = 0; i < instructions.size(); i++){
        std::cout << "\nINSTRUCTIONS FOR PROCESS" << i+1 << std::endl;
        printVector(instructions[0]);
    }
    


    return 0;
}