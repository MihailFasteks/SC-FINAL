#define WIN32_LEAN_AND_MEAN

#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <queue>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 4096

#define SERVER_IP "127.0.0.1"
#define DEFAULT_PORT "8888"

SOCKET client_socket;
bool Wait = false;

// Очередь для хранения сообщений от клиентов
queue<string> messages_queue;

bool isOrderInProgress = false;

DWORD WINAPI Sender(void* client_socket_ptr)
{

    while (true)
    {
        char query[DEFAULT_BUFLEN];
        cin.getline(query, DEFAULT_BUFLEN);
        send(client_socket, query, strlen(query), 0);

    }
}
DWORD WINAPI Receiver(void* param) 
{
    while (true) 
    {
        char response[DEFAULT_BUFLEN];
        int result = recv(client_socket, response, DEFAULT_BUFLEN, 0);
        if (result == SOCKET_ERROR) {
            // Обработка ошибки при приеме сообщения
            return 1;
        }
        response[result] = '\0';

        //(время ожидания)
        istringstream stream(response);
        int totalCookTime = 0;
        double price = 0;
        string word;
        int i = 0;
        
        while (stream >> word) {
            if (i == 0) {
                totalCookTime = stoi(word);
            }
            if (i == 1) {
                price = stod(word);
            }
            i++;
        }
        //int totalCookTime = atoi(response);

        if (totalCookTime == 0)
        {
            cout << "Sorry, we do not have this in our menu" << endl;
        }
        else  
        {
            // Вывод информации о времени ожидания
            cout << "Estimated wait time: " << totalCookTime << " seconds" << endl;

            //Ожидание сообщения от сервера о выполнении заказа
            char resp[200];
            result = recv(client_socket, resp, 200, 0);
            if (result == SOCKET_ERROR) {
                // Обработка ошибки при приеме сообщения о цене
                return 1;
            }
            resp[result] = '\0';


            cout << resp << endl;

            //double price;
            //price = stod(priceBuffer);
            cout << "You need to pay: " << price << endl;
        }
        
    }
}

int main() 
{
    system("title Client");

    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) 
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    addrinfo hints = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    addrinfo* result = nullptr;
    iResult = getaddrinfo(SERVER_IP, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 2;
    }

    addrinfo* ptr = nullptr;
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) 
    {
        client_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (client_socket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 3;
        }
        iResult = connect(client_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(client_socket);
            client_socket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (client_socket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 4;
    }


    string menu;
    ifstream in("menu.txt", ios::in | ios::binary);
    if (!in)
    {
        cout << "Problem with file!" << endl;
        return 1;
    }
    else
    {
        string line;
        while (getline(in, line)) 
        {
            menu += line + "\n";
        }
    }
        
    
    cout << "Welcome to our cafe!" << endl;
    cout << menu << endl << endl;
    
    cout << "What do you want to order?" << endl;


    CreateThread(0, 0, Sender, 0, 0, 0);
    CreateThread(0, 0, Receiver, 0, 0, 0);

    Sleep(INFINITE);

    return 0;
}
