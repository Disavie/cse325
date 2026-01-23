#include <iostream>
#include <string>
#include <fcntl.h>    // open(), O_* flags
#include <unistd.h>   // read(), write(), close()
#include <sys/types.h> // ssize_t, size_t

void invalid(std::string msg){
            std::cout << "Invalid input: " << msg<< std::endl;
            exit(1);
}

int main(int argc, char* argv[]){
        int argsCount = 1;
        std::cout << argc << std::endl;

        if (argc == 1){
            invalid("no arguments were provided");
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
            if (param[0] == '-'){   /// < If it is a command
                switch (param[1]){
                    case 't':
                        trunc = true;
                        break;
                    case 'a':
                        append = true;
                        break;
                    case 'b':
                        //Edge case if -b arg given but no size follows
                        argsCount++;
                        try{
                            samplesz = std::stoi(std::string(argv[argsCount]));
                        }catch(const std::invalid_argument& e){
                            invalid("No sampling size or invalid sampling size was provided");
                        }
                        break;

                    default:
                        invalid("no such command exists");
                }
            }else{  /// < Treat it as a file name if not a command
                /*
                *Exactly two file names must be provided by the user, with the source file listed first and the
                *destination file listed second
                */
                if(file1 == ""){
                    file1 = param;
                }else if (file2 == ""){
                    file2 = param;
                }else{
                    invalid("too many file names were requested");
                }
            }
            argsCount++;
        }
        std::cout << trunc << "\n" << append << "\n" << samplesz << "\n" << file1 << "\n" << file2 << std::endl;

        /*
        *   Step 2: transfer contents of file1 to file2 following constraints of any
        *   commands present
        */

        /* FUNCTIONS
        * int open( const char *pathname, int flags, mode t mode )
        * int close( int fd )
        *
        * ssize t read( int fd, void *buf, size t count )
        * ssize t write( int fd, const void *buf, size t count );
        */

        /*
        *   Opening file to be read
        */
        int read_file;
        read_file = open(file1.c_str(), O_RDONLY);
        if (read_file == -1){
            invalid("file to open does not exist");
        }
        /*
        *   Opening file to be written to
        */
        int write_file;
        int flags = O_CREAT | O_WRONLY;
        if (trunc == true){
            flags |= O_TRUNC;
        }
        if (append == true){
            flags |= O_APPEND;
        }

        write_file = open(file2.c_str(), flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        
        char buffer[samplesz]; /// < Buffer to be read to and written from
        ssize_t bytesRead;
        while((bytesRead = read(read_file,buffer,samplesz)) > 0)
            ssize_t bytesWritten = write(write_file,buffer,bytesRead);
        return 0;

        close(read_file);
        close(write_file);

}