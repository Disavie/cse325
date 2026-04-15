#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string>
#include <vector>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <algorithm>
#include <random>
#include <sstream>
#include <iomanip>
#include <filesystem>

#define BUFFER_SIZE 1024
#define BACKLOG 5

static const size_t AES_KEYLEN = 32;
static const size_t AES_GCM_IVLEN = 12;
static const size_t AES_GCM_TAGLEN = 16;



const unsigned char KEY[AES_KEYLEN] = {
    0xff,0xac,0xbb,0x8c,0x90,0x11,0x90,0x22,
    0xea,0xfe,0xcb,0x54,0x96,0x76,0x80,0x90,
    0xae,0xbc,0x6b,0xfc,0x0f,0xaa,0x37,0x98,
    0xbb,0x99,0x89,0x33,0x06,0xf4,0x9f,0xaf
};


struct chunk
{
    std::vector<unsigned char> cyphertext;
    std::vector<unsigned char> tag;
    std::vector<unsigned char> nonce;
};


std::string toHexString(const std::vector<unsigned char>& data) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');

    for (unsigned char c : data) {
        oss << std::setw(2) << static_cast<int>(c);
    }

    return oss.str();
}

//generate nonce
std::vector<unsigned char> generateNonce() {
    const size_t ivSize = AES_GCM_IVLEN; // 96-bit IV for AES-GCM
    std::vector<unsigned char> iv(ivSize);

    if (RAND_bytes(iv.data(), ivSize) != 1) {
        throw std::runtime_error("RAND_bytes failed to generate IV");
    }

    return iv;
}

std::string get_server_ip() {
    char hostname[BUFFER_SIZE];
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        std::cerr << "Error retrieving hostname.\n";
        return "UNKNOWN";
    }

    struct hostent* host = gethostbyname(hostname);
    if (host == nullptr || host->h_addr_list[0] == nullptr) {
        std::cerr << "Error resolving IP address.\n";
        return "UNKNOWN";
    }

    return inet_ntoa(*(struct in_addr*)host->h_addr_list[0]);
}

ssize_t sendChunk(int client_socket, std::string chunk){
    const char *p = (const char*)chunk.data();
    size_t remaining = chunk.size();
    while (remaining > 0) {
        ssize_t nw = send(client_socket, p, remaining, 0);
        if (nw <= 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        remaining -= nw;
        p += nw;
    }
    return chunk.size();
}

bool aes_gcm_encrypt(
    const std::vector<unsigned char>& plaintext,
    const std::vector<unsigned char>& key,
    const std::vector<unsigned char>& nonce,
    std::vector<unsigned char>& ciphertext,
    std::vector<unsigned char>& tag)
{
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;

    int len;
    int ciphertext_len;

    // Initialize encryption context
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1)
        return false;

    // Set nonce length
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, AES_GCM_IVLEN, nullptr) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    // Set key and nonce
    if (EVP_EncryptInit_ex(ctx, nullptr, nullptr, key.data(), nonce.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    // Encrypt plaintext
    ciphertext.resize(plaintext.size());
    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext.size()) != 1)
        return false;
    ciphertext_len = len;

    // Finalize encryption
    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1)
        return false;
    ciphertext_len += len;
    ciphertext.resize(ciphertext_len);

    // Get the authentication tag
    tag.resize(AES_GCM_TAGLEN);
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, AES_GCM_TAGLEN, tag.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    EVP_CIPHER_CTX_free(ctx);
    return true;
}


void handle_client(int client_socket) {
    char filename[BUFFER_SIZE] = {0};
    int bytes_received = recv(client_socket, filename, BUFFER_SIZE, 0);

    bool fileb = false;
    bool filec = false;

    if (bytes_received <= 0) {
        std::cerr << "Error receiving filename.\n";
        close(client_socket);
        return;
    }

    //std::cout << std::string(filename) << std::endl;

    if (strcmp(filename, "PING") == 0){
        if (send(client_socket, "PONG", 4, 0) != 4) {
            std::cerr << "Error sending PONG.\n";
        }
        return;
    } 

    if (strcmp(filename, "fileB") == 0){
        fileb = true;
    } 

    if (strcmp(filename, "fileC") == 0){
        filec = true;
    } 

    //check if path is a directory
    std::filesystem::path path = filename; // Replace with your path

    // Check if my_directory is a directory
    std::error_code ec;
    if (std::filesystem::is_directory(path, ec)) {
        if (send(client_socket, "FAIL", 4, 0) != 4) {
            std::cerr << "Error sending FAIL.\n";
        }
        close(client_socket);
        return;
    } 
    if (ec) {
        std::cerr << "Filesystem error: " << ec.message() << "\n";
    }

    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        if (send(client_socket, "FAIL", 4, 0) != 4) {
            std::cerr << "Error sending FAIL.\n";
        }
        close(client_socket);
        return;
    }

    

    // Read the entire file into a single string
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Divide into 5 parts
    std::vector<std::string> parts;
    size_t total = content.size();
    size_t partSize = total / 5;

    for (int i = 0; i < 5; ++i) {
        size_t start = i * partSize;
        size_t end = (i == 4) ? total : (i + 1) * partSize;
        parts.push_back(content.substr(start, end - start));
    }

    //get key as vector
    size_t keySize = sizeof(KEY) / sizeof(KEY[0]);
    std::vector<unsigned char> keyVec(KEY, KEY + keySize);

    std::vector<unsigned char> ciphertext, tag;

    std::vector<chunk> chunksEncrypted;


    // Encrypt the 5 partitions
    for (int i = 0; i < 5; ++i) {
        chunk temporalChunk;
        
        std::vector<unsigned char> nonce;
        try {
            nonce = generateNonce();
        } catch (const std::exception& e) {
            std::cerr << "Error generating nonce: " << e.what() << "\n";
            close(client_socket);
            return;
        }
        //std::vector<unsigned char> nonce(AES_GCM_IVLEN);

        std::vector<unsigned char> plaintext(parts[i].begin(), parts[i].end());
        if (!aes_gcm_encrypt(plaintext, keyVec, nonce, ciphertext, tag)) {
            std::cerr << "Encryption failed!\n";
            close(client_socket);
            return;
        }
        temporalChunk.cyphertext = ciphertext;
        temporalChunk.tag = tag;
        temporalChunk.nonce = nonce;
        chunksEncrypted.push_back(temporalChunk);
    }

    //get random order
    std::vector<int> order = {0, 1, 2, 3, 4};
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(order.begin(), order.end(), gen);


    //send like [order]-[nonce]-[ciphertext]-[tag] 
    try {
        for (int i = 0; i < 5; ++i) {
            std::string temp = "";
            temp += std::to_string(order[i]);
            temp += "-";
            // wrong nonce if fileb is selected
            if(fileb){
                std::vector<unsigned char> wrongNonce = generateNonce();
                temp += toHexString(wrongNonce);
            }else{
                temp += toHexString(chunksEncrypted[order[i]].nonce);
            }
            temp += "-";
            // wrong cyphertext
            if(filec){
                std::string wrongCypherText = toHexString(chunksEncrypted[order[i]].cyphertext);
                std::reverse(wrongCypherText.begin(), wrongCypherText.end());
                temp += wrongCypherText;
            }else{
                temp += toHexString(chunksEncrypted[order[i]].cyphertext);
            }
            temp += "-";
            temp += toHexString(chunksEncrypted[order[i]].tag);
            temp += "\n";
            if (sendChunk(client_socket, temp) < 0) {
                std::cerr << "Error sending chunk.\n";
                break;
            }
            //std::cout << temp << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error during sending: " << e.what() << "\n";
        close(client_socket);
        return;
    }

    file.close();
    close(client_socket);
}

int main(int argc, char *argv[]){
    int port = 50003;

    if (argc >= 2)
    {
        port = std::stoi(argv[1]);
    }

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Error creating socket.\n";
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(50003);

    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error binding socket.\n";
        close(server_socket);
        return 1;
    }

    if (listen(server_socket, BACKLOG) < 0) {
        std::cerr << "Error listening on socket.\n";
        close(server_socket);
        return 1;
    }

    std::string server_ip = get_server_ip();
    if (server_ip != "UNKNOWN") {
        std::cout << server_ip << " " << ntohs(server_addr.sin_port) << "\n";
    } else {
        std::cerr << "Unable to determine server IP address.\n";
    }

    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            std::cerr << "Error accepting connection.\n";
            continue;
        }
        handle_client(client_socket);
    }

    close(server_socket);
    return 0;
}
