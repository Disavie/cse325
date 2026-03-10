#include <openssl/evp.h>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <unistd.h>
#include <filesystem>
#include <vector>

using namespace std;

struct Item{
    string path;
    string hash;
    int sz;
    

};

void err(string msg){
    cout << msg << endl;
}

string readFile(string filename){
    ifstream handler(filename,ios::binary);
    if (!handler) {
        err("problem opening file:");
        err(filename);
        return "";
    }

    string s((istreambuf_iterator<char>(handler)),
            istreambuf_iterator<char>());
    handler.close();
    return s;
}

/// Byte array to hex string
string bth(const unsigned char* bytes, size_t l) {
    stringstream ss;

    ss << std::hex << std::setfill('0');

    for (size_t i = 0; i < l; ++i) {
        ss << std::setw(2) << static_cast<unsigned int>(bytes[i]);
    }

    return ss.str();
}
// Computes SHA256 hash of input string and returns hex digest
string sha256_hash(const string& input) {

    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len = 0;

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();  // create context

    EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);  // initialize SHA256
    EVP_DigestUpdate(ctx, input.c_str(), input.size());  // add data
    EVP_DigestFinal_ex(ctx, digest, &digest_len);  // finalize hash

    EVP_MD_CTX_free(ctx);  // free context

    return bth(digest, digest_len);
}

vector<Item> searchDirectory(string path){
    vector<Item> entries;

    
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_directory()){
            string subdir = path;
            subdir = subdir + "/" + entry.path().filename().string();
            cout << subdir << endl;
            vector<Item> subdirEntries = searchDirectory(subdir);
            //Append to entries
            for(Item s : subdirEntries){
                entries.push_back(s);
            }
        }else{
            string hash =sha256_hash(readFile(entry.path().string()));
            entries.push_back(Item{
                        entry.path().string(),
                        hash, // todo
                        (int)entry.file_size()
                        });
        }
    }
    return entries;

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
    string dir = "";
    int mode = -1;
    enum Mode{Init, Check};
    for (int i = 1; i < argc; i++) {
        string s(argv[i]);
        if (s == "-d") dir = argv[++i];
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
    if (dir == "") err("bad directory");
    if (mode == -1) err("neither -i nor -c was provided");

    cout << "Target Directory: " << dir << endl;

    // 2 - Walk directory tree & subdirectories
    /**
     * When scanning the target directory:
     * 1. The scan must include all regular files in the directory and its subdirectories.
     * 2. Ignore directories and ignore symbolic links.
     * 3. The program must not crash if it encounters a file it cannot open. It must record the error in the
     * report (check mode) or print an error message (init mode) and continue scanning other files.
     * 4. If the manifest file or report file is located inside the target directory, your program must exclude
     * those files from the scan.
     */
    
    /// int pid = fork();
    // Read all things in target directory
    auto res = searchDirectory(dir);
    for(Item s : res)
        cout << s.sz << " " << s.hash << " " << s.path << endl;
    return 0;
}
