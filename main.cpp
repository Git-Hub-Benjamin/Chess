#include <iostream>
using namespace std;
int testing[8][8] = {0};

void printboard(){
    for(int i = 0; i < 8; i++){
        for(int j = 0; j < 8; j++){
            cout << "| " << testing[i][j] << " ";
        }
        cout << "|" << endl;
    }
}
int main() {

    testing[0][1] = 1;
    printboard(); 
}
