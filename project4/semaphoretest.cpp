#include <iostream>
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

using namespace std;

sem_t sem;

void * doSomething(void * arg){
    sem_wait(&sem);
    //sem count = -1
    
    cout << "doing something" << endl;
    return nullptr;
}

void * doSomethingElse(void * arg){
    cout << "Doing Something Else" << endl;

    sem_post(&sem);
    // sem count = 0 -> allows another to enter critical section
    return nullptr;
}
int main(int argc, char ** argv){

    // sem_t *sem, pshared, value
    sem_init(&sem,0,0);
    pthread_t t1,t2;

    pthread_create(&t1,nullptr,doSomething,nullptr);
    sleep(1);
    //Even though thread 2 always will get dispatched after thread 1, doSomethingElse will execute first ( i hope )
    pthread_create(&t2,nullptr,doSomethingElse,nullptr);

    pthread_join(t1, nullptr);
    pthread_join(t2, nullptr);

    sem_destroy(&sem);


    return 0;

}

