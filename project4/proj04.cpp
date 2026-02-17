#include <iostream>
#include <vector>
#include <string>
#include <pthread.h>
#include <unistd.h>
#include <fstream>
#include <sstream>

using namespace std;


struct Object{
    unsigned int id;
    float price;
    unsigned int quantity;
    string description;
};

void PrintInventory(vector<Object> &Inventory){
    // Print objects in Inventory
    for (size_t i = 0; i < Inventory.size(); i++) {
        cout << Inventory[i].id << " "
            << Inventory[i].price << " "
            << Inventory[i].quantity << " "
            << Inventory[i].description << endl;
    }
}

void * InitProducer(void * arg){
    cout << '[' << *(int *)arg << ']' << endl;
    return nullptr;
}

void * InitConsumer(void * arg){
    cout << "Consumer!" << endl;
    return nullptr;
}


int main(int argc, char ** argv){

    const int DEFAULTSZ = 10; 
    vector<Object> Inventory(DEFAULTSZ);
    int numProducer = 1;

    for(int i = 0 ; i < argc ; i++){
        string s(argv[i]);
        //Deal with commands -b -> set buffer size default = 10
        //Default number of producers = 1 max 9

        if(s[0] == '-'){
            switch(s[1]){
                case 'b':
                    Inventory.resize(stoi(argv[++i]));
                    break;
                case 'p':
                    numProducer = stoi(argv[++i]);
                    break;
                default:
                    cout << "err : unknown option " << s[1] << endl;
            }
        }
    }
    
    cout << "inventory size = " << Inventory.size() << endl;
    cout << "numProducer = " << numProducer << endl;

    //init inventory
    string filename = "testcases/inventory.old";
    ifstream inventory;
    inventory.open(filename);
    string line;
    int i = 0;
    while (getline(inventory, line)) {
        stringstream ss(line);

        int id;
        double price;
        int quantity;
        string description;

        ss >> id >> price >> quantity;
        getline(ss, description);          // get rest of line as description

        // remove leading space from description
        if (!description.empty() && description[0] == ' ')
            description.erase(0, 1);

        Object x;
        x.id = id;
        x.price = price;
        x.quantity = quantity;
        x.description = description;

        Inventory[i] = x;
        i++;
    }

    inventory.close();

    PrintInventory(Inventory);
    /// Add 1 to account for singular consumer (order fulfiller)
    pthread_t threads[1+numProducer];

    ///dispatch consumer
    pthread_create(&threads[0],nullptr,InitConsumer,nullptr);

    //init producers
    vector<int> ids(numProducer);
    for (int i = 0; i < numProducer; i++) {
        ids[i] = i;
        pthread_create(&threads[i + 1], nullptr, InitProducer, &ids[i]);
    }

    /// Rejoin threads
    for(int i = 0 ; i < 1+numProducer; i++){
        /// Rejoin threads
        pthread_join(threads[i],nullptr);
    }
    return 0;
}
