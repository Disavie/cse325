#include <iostream>
#include <vector>
#include <string>
#include <pthread.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <semaphore.h>

using namespace std;

struct Order {
    unsigned int customer_id = 0;
    unsigned int product_id = 0;
    unsigned int quantity = 0;
    bool done = false; // signals producer finished
};

struct Item {
    unsigned int product_id = 0;
    double price = 0.0;
    unsigned int quantity = 0;
    string description;
};

vector<Item> Inventory;     // real inventory
vector<Order> Buffer;       // bounded buffer

int buffer_size = 10;
int in = 0;
int out = 0;

int numProducers = 1;
int finishedProducers = 0;

sem_t mutex;
sem_t sem_empty;
sem_t sem_full;

void* Producer(void* arg) {
    int id = *(int*)arg;
    string filename = "orders" + to_string(id + 1);

    ifstream file(filename);
    string line;

    while (getline(file, line)) {
        stringstream ss(line);
        Order order;
        ss >> order.customer_id >> order.product_id >> order.quantity;

        sem_wait(&sem_empty);
        sem_wait(&mutex);

        Buffer[in % buffer_size] = order;
        in++;

        sem_post(&mutex);
        sem_post(&sem_full);
    }

    // Send termination record
    Order doneOrder;
    doneOrder.done = true;

    sem_wait(&sem_empty);
    sem_wait(&mutex);

    Buffer[in % buffer_size] = doneOrder;
    in++;

    sem_post(&mutex);
    sem_post(&sem_full);

    return nullptr;
}

void* Consumer(void* arg) {

    ofstream log("log");

    while (finishedProducers < numProducers) {

        sem_wait(&sem_full);
        sem_wait(&mutex);

        Order order = Buffer[out % buffer_size];
        out++;

        sem_post(&mutex);
        sem_post(&sem_empty);

        if (order.done) {
            finishedProducers++;
            continue;
        }

        // Find product in inventory
        bool filled = false;
        double transaction = 0.0;
        string description = "Unknown";

        for (auto& item : Inventory) {
            if (item.product_id == order.product_id) {
                description = item.description;

                if (item.quantity >= order.quantity) {
                    item.quantity -= order.quantity;
                    transaction = item.price * order.quantity;
                    filled = true;
                }
                break;
            }
        }

        log << left
            << setw(10) << order.customer_id
            << setw(8)  << order.product_id
            << setw(20) << description
            << setw(6)  << order.quantity
            << "$" << fixed << setprecision(2) << transaction
            << " "
            << (filled ? "filled" : "rejected")
            << endl;
    }

    log.close();
    return nullptr;
}

// ================= MAIN =================
int main(int argc, char** argv) {

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        string s(argv[i]);
        if (s == "-b") buffer_size = stoi(argv[++i]);
        if (s == "-p") numProducers = stoi(argv[++i]);
    }

    // Load inventory
    ifstream inventoryFile("inventory.old");
    string line;

    while (getline(inventoryFile, line)) {
        stringstream ss(line);
        Item item;

        ss >> item.product_id >> item.price >> item.quantity;
        getline(ss, item.description);

        if (!item.description.empty() && item.description[0] == ' ')
            item.description.erase(0, 1);

        Inventory.push_back(item);
    }

    inventoryFile.close();

    // Initialize buffer
    Buffer.resize(buffer_size);

    sem_init(&mutex, 0, 1);
    sem_init(&sem_empty, 0, buffer_size);
    sem_init(&sem_full, 0, 0);

    pthread_t threads[numProducers + 1];

    // Start consumer
    pthread_create(&threads[0], nullptr, Consumer, nullptr);

    // Start producers
    vector<int> ids(numProducers);
    for (int i = 0; i < numProducers; i++) {
        ids[i] = i;
        pthread_create(&threads[i + 1], nullptr, Producer, &ids[i]);
    }

    // Join all threads
    for (int i = 0; i < numProducers + 1; i++)
        pthread_join(threads[i], nullptr);

    // Write updated inventory
    ofstream newInventory("inventory.new");
    for (const auto& item : Inventory) {
        newInventory << item.product_id << " "
                     << fixed << setprecision(2) << item.price << " "
                     << item.quantity << " "
                     << item.description << endl;
    }
    newInventory.close();

    sem_destroy(&mutex);
    sem_destroy(&sem_empty);
    sem_destroy(&sem_full);

    return 0;
}

