#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <winsock2.h>
#include <windows.h>
#include "heap_manager.h"

#define SERVER_PORT 4010
#define BUFFER_SIZE 1024

CRITICAL_SECTION heapCriticalSection;
int allocationCounter = 0;
int deallocationCounter = 0;
double fragmentationDegree = 0.0;

typedef struct {
    SOCKET clientSocket;
} ThreadData;

void* _alloc(int size) {
    void* ret = NULL;

    EnterCriticalSection(&heapCriticalSection);

    ret = allocate_memory(size);

    LeaveCriticalSection(&heapCriticalSection);

    return ret;
}

void _free(void* address) {
    EnterCriticalSection(&heapCriticalSection);

    free_memory(address);

    LeaveCriticalSection(&heapCriticalSection);
}

void printData() {
    printf("Number of Allocations: %d\n", allocationCounter);
    printf("Number of Deallocations: %d\n", deallocationCounter);
    printf("Fragmentation Degree: %.2f%%\n", fragmentation_degree());
}

DWORD WINAPI ClientThread(LPVOID lpParam) {
    ThreadData* threadData = (ThreadData*)lpParam;
    SOCKET clientSocket = threadData->clientSocket;
    char buffer[BUFFER_SIZE];
    int iResult;

    do {
        memset(buffer, 0, sizeof(buffer));

        // Receive data from the client
        iResult = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (iResult > 0) {
            printf("Received data from client: %s\n", buffer);

            if (strcmp(buffer, "exit") == 0) {
                printf("Client requested to exit. Closing the server.\n");
                break;
            }

            if (strncmp(buffer, "allocate", 8) == 0) {
                // Parse the size from the request
                int size = atoi(buffer + 9);

                // Allocate memory
                unsigned int ret = (unsigned int)_alloc(size);
                allocationCounter++;
                printData();

                // Send the response back to the client
                if (ret != 0) {
                    itoa(ret, buffer, 10);
                    // Send the success response back to the client
                    send(clientSocket, buffer, 28, 0);
                } else {
                    // Send the failure response back to the client
                    send(clientSocket, "Memory allocation failed", 24, 0);
                }
            } else if (strncmp(buffer, "free", 4) == 0) {
                // Parse the address from the request
                void* address = (void*)atoi(buffer + 5);

                // Free memory
                _free(address);
                if(freeMemoryResult != 0)
                {
                    deallocationCounter++;
                    printData();

                    LeaveCriticalSection(&heapCriticalSection);

                    // Send the response back to the client
                    send(clientSocket, "Memory freed successfully", 24, 0);
                }else{
                    send(clientSocket, "Attempted to free already freed memory!", 38, 0);
                }
            } else {
                // Invalid request
                send(clientSocket, "Invalid request", 15, 0);
            }
        } else if (iResult == 0) {
            printf("Client disconnected.\n");
            break;
        } else {
            printf("recv failed with error: %d\n", WSAGetLastError());
            break;
        }
    } while (1);

    // Cleanup
    closesocket(clientSocket);
    free(threadData);

    return 0;
}

int main() {
    WSADATA wsaData;
    SOCKET serverSocket;
    struct sockaddr_in serverAddress, clientAddress;
    int clientAddressLength = sizeof(clientAddress);
    int iResult;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return 1;
    }

    // Create a socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        printf("socket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Initialize server address structure
    memset((char*)&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(SERVER_PORT);

    // Bind the socket
    iResult = bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    iResult = listen(serverSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    printf("Server is waiting for connections...\n");

    InitializeCriticalSection(&heapCriticalSection);

    // Main loop for handling client connections
    do {
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
        if (clientSocket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        printf("Client connected.\n");

        // Create a new thread data structure
        ThreadData* threadData = (ThreadData*)malloc(sizeof(ThreadData));
        threadData->clientSocket = clientSocket;

        // Create a new thread to handle the client
        HANDLE clientThread = CreateThread(NULL, 0, ClientThread, (LPVOID)threadData, 0, NULL);

        if (clientThread == NULL) {
            printf("Error creating client thread.\n");
            closesocket(clientSocket);
            free(threadData);
            break;
        }

        // Close the thread handle (thread will continue running)
        CloseHandle(clientThread);

    } while (1);
    // Cleanup
    closesocket(serverSocket);
    WSACleanup();
    DeleteCriticalSection(&heapCriticalSection);

    return 0;
}
