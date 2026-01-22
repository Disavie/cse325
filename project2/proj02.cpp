#include <iostream>
#include <string>

void invalid(){
            std::cout << "Invalid input" << std::endl;
            exit(1);
}

int main(int argc, char* argv[]){
        int i = 0;
        int argsCount = 1;
        std::cout << argc << std::endl;

        if (argc == 1){
            invalid();
        }

        /*
        *   Step 1: Parse cmd line arguments;
        *   save file1, file2, and any commands to variables
        *
        */
        
        bool trunc = false;
        bool append = false;
        int samplesz = 64;
        std::string file1;
        std::string file2;



        while (argsCount < argc){
            std::string param = argv[argsCount];
            if (param == "-t"){
                    trunc = true;
            }else if ( param == "-a"){
                    append = true;
            }else if(param == "-b"){
                //Edge case if -b arg given but no size follows
                argsCount++;
                try{
                    samplesz = std::stoi(std::string(argv[argsCount]));
                }catch(std::invalid_argument e){
                    invalid();
                }
            }else{
                //Edge case of more/less than 2 non-command arguements are passed.. handle this..
                if(file1 == ""){
                    file1 = param;
                }else{
                    file2 = param;
                }
            }
            argsCount++;
        }
        std::cout << trunc << "\n" << append << "\n" << samplesz << "\n" << file1 << "\n" << file2 << std::endl;

        /*
        *   Step 2: transfer contents of file1 to file2 following constraints of any
        *   commands present
        *
        */
        return 0;


}