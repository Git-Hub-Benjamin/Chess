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
#include <random>
#include "server.h"

#define MAX_CLIENT 8
std::mutex mtx;
static Client* clients[MAX_CLIENT] = { nullptr };

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

void client_access_lock() { mtx.lock(); }
void client_access_unlock() { mtx.unlock(); }



void client_dispatcher(){
    client_access_lock();
    int ready_clients = 0;
    int ready_client_one_index = -1;
    int ready_client_two_index = -1;

    for(int client_index = 0; client_index < MAX_CLIENT; client_index++){
        if(clients[client_index] != nullptr && clients[client_index]->status == WAITING){
            // found waiting client, increase count
            ready_clients++;
            if(ready_client_one_index < 0)
                ready_client_one_index = client_index;
            else
                ready_client_two_index = client_index;
        }
        if(ready_clients == 2){
            // send these two clients to a game
            clients[ready_client_one_index]->status = PLAYING;
            clients[ready_client_two_index]->status = PLAYING;

            //! ADD NEW GAME, idk if i should pass Client or ChessClient
            //std::thread new_game(clients[ready_client_one_index], clients[ready_client_two_index]);
            ready_clients = 0;
            ready_client_one_index = -1;
            ready_client_two_index = -1;
        }
    }
    client_access_unlock();
}

void close_client(int index){
    client_access_lock();
    delete clients[index];
    client_access_unlock();
}

int add_client(int client_fd){
    client_access_lock();
    int res = -1;
    for(int client_index = 0; client_index < MAX_CLIENT; client_index++){
        if(clients[client_index] == nullptr){
            // found an open position
            clients[client_index] = new Client{client_fd, client_index};
            res = 0;
            break; // need to unlock!
        }
    }
    client_access_unlock();
    return res;
}

void waiting_room(){
    // random num setup
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);
    
    // tracking 
    int oldnum;
    int newnum = dis(gen);
    while(true){
        oldnum = newnum;
        newnum = dis(gen);
        
        for(int client_index; client_index < MAX_CLIENT; client_index++){

        }

        std::this_thread::sleep_for(std::chrono::seconds(3));
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

    if(listen(serverSocketFD, MAX_CLIENT) != 0){
        std::wcout << "Error listening." << std::endl;
        exit(EXIT_FAILURE);
    }

    // thread for waiting room
    //std::thread wroom(waiting_room);

    // Server is setup now...

    while(true)
    {
        struct sockaddr_in* clientAddress;
        socklen_t client_addr_len = sizeof(*clientAddress);
        int clientSocketFD;
        if((clientSocketFD = accept(serverSocketFD, (struct sockaddr*)&clientAddress, &client_addr_len)) < 0){
            std::wcout << "Accept failed." << std::endl;
            continue;
        }

        std::cout << "Accpeted." << std::endl;

        char buffer[1024];

        while (true) {
            int bytesReceived = recv(clientSocketFD, buffer, 1024, 0);
            std::cout << "Bytes received: " << bytesReceived << std::endl;
            if (bytesReceived <= 0) {
                // Handle disconnection or error
                std::cout << "Connection closed." << std::endl;
                break;
            }
            buffer[bytesReceived] = '\0'; // Null-terminate the received message
            std::cout << "Message from client: " << buffer << std::endl;
            std::string message = "From server";
            send(clientSocketFD, message.c_str(), message.length(), 0);
            std::cout << "Sent message." << std::endl;
        }



        // add client to server
        int res = add_client(clientSocketFD);
        if(res < 0){
            // technically this should never happen it would fail higher up
            std::wcout << L"Could not find free thread" << std::endl;
            // idk if we should fail or just continue lol
        }
        // after adding client call dispatcher
        client_dispatcher();

        continue;
    }
    
    std::wcout << "Sever starting." << std::endl;
    //int socketFD = socket()

    std::string custom = "Benjamin";
    // create json object
    // cJSON *root = cJSON_CreateObject();
    // cJSON_AddStringToObject(root, "name", custom.c_str());
    // cJSON_AddNumberToObject(root, "age", 30);

    // // convery the json object into a string, but we cant do directly to wstring
    // std::wstring myWstring = charP_to_wstring(cJSON_Print(root));   
    // std::string myString = cJSON_Print(root);

    // std::wcout << "Json object --> " << myWstring << std::endl;

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
