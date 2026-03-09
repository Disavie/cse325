#include <openssl/evp.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;

void err(string msg){
    cout << msg << endl;
}

int main(int argc, char ** argv){

    // Parse arguments
    /**
     * The supported arguments are:
     * 1. -i Initialize baseline mode (create a manifest).
     * 2. -c Check mode (compare current files against the manifest).
     * 3. -d <dir> Target directory to scan (required for both modes).
     * 4. -m <file> Manifest filename. Default is baseline.manifest.
     * 5. -r <file> Report filename (check mode only). Default is report.txt.
     * Exactly one of -i or -c must be provided.
     */

    string manifestname = "baseline.manifest";
    string reportname = "report.txt";
    string dir = ".";
    int mode = -1;
    enum Mode{Init, Check};
    for (int i = 1; i < argc; i++) {
        string s(argv[i]);
        if (s == "-d") dir = dir+"/"+argv[++i];
        if (s == "-m") manifestname = argv[++i];
        if (s == "-r" && mode == Mode::Check) reportname = argv[++i];
        if (s == "-c"){
            if(mode == -1){
                mode = Mode::Check; i++; 
            }else{
                err("exactly one of -i -c expected");
            }
        }
        if (s == "-i"){ 
            if(mode == -1){
                    mode = Mode::Init; i++; 
            }else{
                err("exactly one of -i -c expected");
                    
            }
        }
    }
    cout << dir << endl;


    return 0;
}
