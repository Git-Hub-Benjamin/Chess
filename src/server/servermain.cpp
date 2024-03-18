#include "../chess.h"
#include "./socket/socketutil.h"
#include "../../jsonlib/cJSON.h"
#include <stdio.h>
#include <string>
#include <pthread.h> // may user thread
#include <thread>
#include <mutex>
#include <time.h>
#include <unistd.h>

#define MAX_PLAYER 8

void handleOption(){
    // this should be a client only function so we need to refactor
}

std::wstring charP_to_wstring(const char* str){
    std::string temp = str;
    return std::wstring(temp.begin(), temp.end());
}

std::wstring string_to_wstring(std::string &str){
    return std::wstring(str.begin(), str.end());
}

enum THREAD_STATUS{
    WAITING,
    PLAYING
};

class ChessClient{
public:
    int clientFD;
    int ThreadIndex;
    enum THREAD_STATUS status;

    ChessClient(int fd, int index): clientFD(fd), ThreadIndex(index) {}
};

std::mutex mtx;
static ChessClient* active_clients[MAX_PLAYER] = { nullptr };

void active_clients_add(ChessClient* setClient, int index){
    mtx.lock();
    active_clients[index] = setClient;
    mtx.unlock();
}

void player_waiting(ChessClient* client){

    while(true){
        if(client->status == PLAYING){
            
            break;
        }else{
            // recv from client
        }
    }

    // Game over or disconnected
}

void schedule_clients(){
    while(true)
    {
        int ready_clients = 0;
        int ready_client_one_index = -1;
        int ready_client_two_index = -1;
        for(int i = 0; i < MAX_PLAYER; i++){
            if(active_clients[i] != nullptr && active_clients[i]->status == WAITING){
                // found a waiting client, increase count
                ready_clients++;
                if(ready_client_one_index == -1){
                    ready_client_one_index = i;
                }else{
                    ready_client_two_index = i;
                }
            }
            if(ready_clients == 2){
                ready_clients = 0;
                active_clients[ready_client_one_index]->status = PLAYING;
                active_clients[ready_client_two_index]->status = PLAYING;
                ready_client_one_index = -1;
                ready_client_two_index = -1;
            }
        }
        std::this_thread::yield();
    }
}


//! cJSON functions input and output char*, we will work with std::string
//! When we need to input into a cJSON we will use <stringobj>.c_str(), when
//! we need to print anything to the screen use

int main() {

    // init server
    int serverSocketFD = createTCPIPv4Socket();
    struct sockaddr_in* serverAddress = createIPv4Address("", 2000);

    if(bind(serverSocketFD, (struct sockaddr*)serverAddress, sizeof(*serverAddress)) != 0){
        std::wcout << "Error binding socket." << std::endl;
        exit(EXIT_FAILURE);
    }

    if(listen(serverSocketFD, MAX_PLAYER) != 0){
        std::wcout << "Error listening." << std::endl;
        exit(EXIT_FAILURE);
    }

    // Server is setup now...

    // scheduler thread to run clients
    std::thread client_scheduler(schedule_clients);

    while(true)
    {
        struct sockaddr_in* clientAddress;
        socklen_t client_addr_len = sizeof(clientAddress);
        int clientSocketFD;
        if((clientSocketFD = accept(serverSocketFD, (struct sockaddr*)&clientAddress, &client_addr_len)) < 0){
            std::wcout << "Accept failed." << std::endl;
            continue;
        }

        char buffer[1024];
        recv(clientSocketFD, buffer, 1024, 0);
        buffer[1023] = '\0';

        while(true){
            std::wcout << "Response: " << buffer << std::endl;
            sleep(1); 
        }

        // look through active clients for a free position
        int thread_id = -1;
        for(int client_index = 0; client_index < MAX_PLAYER; client_index++){
            if(active_clients[client_index] == nullptr){
                // found an open position
                thread_id = client_index;
                ChessClient* new_player = new ChessClient{clientSocketFD, thread_id};
                active_clients_add(new_player, thread_id);
                std::thread new_thread(player_waiting, new_player);
                break;
            }
        }
        if(thread_id < 0){
            // technically this should never happen it would fail higher up
            std::wcout << L"Could not find free thread" << std::endl;
            // idk if we should fail or just continue lol
        }

        continue;
    }
    
    std::wcout << "Sever starting." << std::endl;
    //int socketFD = socket()

    std::string custom = "Benjamin";
    // create json object
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "name", custom.c_str());
    cJSON_AddNumberToObject(root, "age", 30);

    // convery the json object into a string, but we cant do directly to wstring
    std::wstring myWstring = charP_to_wstring(cJSON_Print(root));   
    std::string myString = cJSON_Print(root);

    std::wcout << "Json object --> " << myWstring << std::endl;

    // // Parse JSON string
    // cJSON *parsedRoot = cJSON_Parse(myString);
    // cJSON *nameItem = cJSON_GetObjectItem(parsedRoot, "name");
    // cJSON *ageItem = cJSON_GetObjectItem(parsedRoot, "age");

    // printf("Name: %s\n", nameItem->valuestring);
    // printf("Age: %d\n", ageItem->valueint);

    // // Cleanup
    // cJSON_Delete(root);
    // cJSON_Delete(parsedRoot);
    // free(jsonString);

    // return 0;
}
