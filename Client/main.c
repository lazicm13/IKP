#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 4010
#define BUFFER_SIZE 100


int main() {
    WSADATA wsaData;
    SOCKET clientSocket;
    struct sockaddr_in serverAddress;
    int iResult;
    char buffer[BUFFER_SIZE];

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return 1;
    }

    // Create a socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        printf("socket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Initialize server address structure
    memset((char*)&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
    serverAddress.sin_port = htons(SERVER_PORT);

    // Connect to the server
    iResult = connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if (iResult == SOCKET_ERROR) {
        printf("connect failed with error: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    printf("Connected to the server.\n");

    // Main loop for sending requests to the server
    do {
        memset(buffer, 0, sizeof(buffer));

        // TODO: Get user input for memory allocation or deallocation
        // and send the appropriate request to the server.

        printf("Enter a request (e.g., 'allocate 100' or 'free 100', 'exit' to quit): ");
        fgets(buffer, sizeof(buffer), stdin);

        // Send the request to the server
        iResult = send(clientSocket, buffer, strlen(buffer), 0);
        if (iResult == SOCKET_ERROR) {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }

        if (strcmp(buffer, "exit\n") == 0) {
            printf("Exiting the client.\n");
            break;
        }

        // Receive a response from the server
        memset(buffer, 0, sizeof(buffer));
        iResult = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (iResult > 0) {
            printf("Server response: %s\n", buffer);
        } else if (iResult == 0) {
            printf("Server disconnected.\n");
            break;
        } else {
            printf("recv failed with error: %d\n", WSAGetLastError());
            break;
        }

    } while (1); // Continue the loop indefinitely

    // Cleanup
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
