#include <iostream>
#include <pthread.h>
#include <unistd.h>

using namespace std;

void * doSomething(void * arg){
   cout << *(int*)arg; 
   delete (int*)arg;
   return nullptr;
}


int main(int argc, char ** argv){

    const int THREADNUM = 10;

    int shared = 0;
    /// Create threads
    pthread_t threads[THREADNUM];
    while(true){
        for(int i = 0 ; i < THREADNUM; i++){
            /// Dispatch threads 
            int * arg = new int(i);
            pthread_create(&threads[i],nullptr,doSomething,arg);
        }

        for(int i = 0 ; i < THREADNUM; i++){
            /// Rejoin threads 
            pthread_join(threads[i],nullptr);
        }
        cout << '\n';
        flush(cout);
        sleep(1);
    }

    return 0;
}
