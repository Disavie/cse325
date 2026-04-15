// client.c
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <string>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <time.h>

#define swap(a,b) do{typeof(*(a))tmp = *a; \
                        *a = *b;\
                        *b = tmp;} while(0)\




typedef struct{
    int order;
    char data[1024];

}packet_t;

typedef struct{
    packet_t packets[5];
    int packet_count;
}msg_t;


void print_array(packet_t * packets, int n) {
    for (int i = 0; i < n; i++)
        printf("%d ", packets[i].order);
    printf("\n");
}


int is_sorted(packet_t * packets, int n){
    print_array(packets,n);
    for(int i = 0 ; i < n - 1; i++){
        if ( packets[i].order > packets[i+1].order ){
            return 0;
        }
    }
    return 1;
}

void shuffle(packet_t * packets, int n){
    //std::cout << "Swapping : ";
    //print_array(packets,n);
    for (int i = 0; i < n ; i++) {
        int j = rand() % n;
        swap(&packets[i], &packets[j]);
    }
    //std::cout << "Done swapping : ";
    //print_array(packets,n);
}


void bogosort(packet_t * packets, int n){

    clock_t start = clock(); 
    while(!is_sorted(packets, n)){
        shuffle(packets, n);
    }
    clock_t end = clock();

    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Bogosorted the packets in %lf seconds\n", time_taken);
}

int main(int argc, char **argv) {
    /// hostname port file-to-send
    if(argc < 4){
        fprintf(stderr,"Usage: %s <hostname> <file>\n", argv[0]);
        return 1;
    }

    char *hostname = argv[1];
    char *port = argv[2];
    char *filename = argv[3];

    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP
                                     //

    int status = getaddrinfo(hostname, port, &hints, &res);
    if(status != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }

    int s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(s == -1){
        perror("socket");
        freeaddrinfo(res);
        return 1;
    }

    if(connect(s, res->ai_addr, res->ai_addrlen) == -1){
        perror("connect");
        freeaddrinfo(res);
        close(s);
        return 1;
    }

    freeaddrinfo(res);

    ssize_t sendsz = send(s, filename, strlen(filename), 0);
    if(sendsz == -1){
        perror("send");
        close(s);
        return 1;
    }
    char buf[1024];
    int packets_recieved = 0;
    int packets_expected = 5;
    msg_t msg;
    while (packets_recieved < packets_expected){
        ssize_t recvlen = recv(s, buf, sizeof(buf)-1, 0);

        if(recvlen == -1){
            perror("recv");
            close(s);
            return 1;
        }

        buf[recvlen] = '\0';
        strcpy(msg.packets[packets_recieved].data,buf);
        msg.packets[packets_recieved].order = int( buf[0] - '0' );
        packets_recieved++;
    }
    msg.packet_count = packets_recieved;


    /// Ok part 1 done
    if (strcmp(msg.packets[0].data,"PONG") == 0){
        printf("good job");
        return 0;
    }else if (strcmp(msg.packets[0].data,"FAIL") == 0){
        std::cerr << "failed to open: " <<  filename << std::endl;
        return 1;
    }
    /// reorder
    bogosort(msg.packets, msg.packet_count);

    for (int i = 0 ; i < sizeof(msg.packets) / sizeof(packet_t) ; i++ ){
        std::cout << msg.packets[i].order << std::endl;
    }


    //std::sort

    /// Decrypt
    ///
    ///[order] - [nonce] - [ciphertext] - [tag]\n
    /*
    • The order number identifies where the packet belongs in the final reconstructed file (from 1 to 5).
    • The nonce is a 12-byte AES-GCM value required to decrypt this specific packet.
    • The ciphertext is the encrypted chunk of the file.
    • The tag is a 16-byte AES-GCM authentication tag used to verify integrity. If it does not match,
    the packet is invalid.
    The server encrypts each chunk using AES-256-GCM. You will be given the shared 32-byte key:

    */
    //const unsigned char KEY[32] = {}

    close(s);
    return 0;
}

