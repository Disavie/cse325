#include <iostream>
#include <vector>
#include <string>
#include <pthread.h>
#include <unistd.h>
#include <fstream>
#include <sstream>

using namespace std;


struct Object{
    unsigned int customer_id = 0;
    unsigned int product_id;
    float price;
    unsigned int quantity;
    string description;
};

#include <iomanip>  // for setw, left, right

void PrintObjects(const vector<Object> &Objects) {
    // Print header
    cout << left
         << setw(12) << "CustomerID"
         << setw(10) << "ProductID"
         << setw(8)  << "Price"
         << setw(10) << "Quantity"
         << "Description" << endl;

    cout << string(60, '-') << endl;  // separator line

    // Print objects
    for (const auto &obj : Objects) {
        cout << left
             << setw(12) << obj.customer_id
             << setw(10) << obj.product_id
             << setw(8)  << fixed << setprecision(2) << obj.price
             << setw(10) << obj.quantity
             << obj.description << endl;
    }
}

void * InitProducer(void * arg){
    //cout << '[' << *(int *)arg << ']' << endl;
    /// Individual order list per producer
    vector<Object> Orders;

    string filename = "orders" + to_string( *(int *)arg+1);
    cout << filename << endl;

    ifstream order;
    order.open(filename);
    string line;
    while (getline(order, line)) {
        stringstream ss(line);

        int c_id;
        int p_id; 
        int quantity;

        ss >> c_id >> p_id>> quantity;


        Object x;
        x.product_id = p_id;
        x.customer_id = c_id;
        x.quantity = quantity;

        Orders.push_back(x);
    }

    order.close();
    
    PrintObjects(Orders);
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
        x.product_id = id;
        x.price = price;
        x.quantity = quantity;
        x.description = description;

        Inventory[i] = x;
        i++;
    }

    inventory.close();

    PrintObjects(Inventory);
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
