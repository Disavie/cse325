// A program calculating address size and memory size from a command line input

#include <iostream>
#include <cmath>    // Used for pow()
#include <cctype>   // Used for tolower and isdigit

int main(int argc, char * argv[]){
//    std::cout << argc << std::endl;
    bool err = false;
    int length = 0;
    if (argc == 2){
        // Getting length of the address
        // Also checking for a valid address, must contain alphanumeric character with largest character being F (ascii 72)
        char *address_ptr = argv[1];
        while(*address_ptr != '\0'){
            //std::cout << int(*c) << std::endl;
            char ch = std::tolower(*address_ptr);
            if(!std::isdigit(ch) && !(ch >= 'a' && ch <= 'f')){
                err = true;
                break;
            } 
            if(length >= 8){
                err = true;
                break;
            }
            length++;
            address_ptr++;
        }
    }else{
        err = true;
    }


    if (!err){
        int bits = 4 * length;
        long memSize = pow(2,bits);
        std::cout << "Address Size: " << bits << " bits" << std::endl;
        std::cout << "Memory Size: " << memSize << " bytes" << std::endl;
    }else{
        // Error case handling
      std::cout << "Error with argv (empty or invalid)" << std::endl;  
    }
    return 0;

}