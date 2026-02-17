#include <iostream>
#include <vector>
#include <string>

using namespace std;


struct Order{

    float price = 0;

};



int main(int argc, char ** argv){

    const int DEFAULTSZ = 10; 
    vector<Order> Inventory(DEFAULTSZ);
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
    



    //init producers


    for (int i = 0 ; i < numProducers < i++){
        //dispatch producer thread and init
    }
    return 0;
}
