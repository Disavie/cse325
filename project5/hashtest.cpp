#include <openssl/evp.h>
#include <openssl/sha.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;


/// Byte array to hex string
string bth(const unsigned char * bytes, size_t l){

    stringstream ss;

    for (size_t i = 0; i < l; ++i) {  
        ss << std::setw(2) << static_cast<int>(bytes[i]);  
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
int main() {  
    std::string input = "hello world";  
    std::string hash = sha256_hash(input);  
 
    std::cout << "Input: " << input << "\n";  
    std::cout << "SHA256 Hash: " << hash << "\n";  
 
    return 0;  
}  
