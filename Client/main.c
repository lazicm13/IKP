#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 4010
#define BUFFER_SIZE 100

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Failed to initialize Winsock");
        return EXIT_FAILURE;
    }

    // Create socket
    SOCKET connectSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (connectSocket == INVALID_SOCKET) {
        printf("Error creating socket");
        WSACleanup();
        return EXIT_FAILURE;
    }

    // Set up server address struct
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
    serverAddress.sin_port = htons(SERVER_PORT);

    // Connect to the server
    if (connect(connectSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        printf("Error connecting to server");
        closesocket(connectSocket);
        WSACleanup();
        return EXIT_FAILURE;
    }

    int iResult;
    char dataBuffer[BUFFER_SIZE];

    // Receive and print the message from the server
    while (strcmp(dataBuffer, "quit") != 0) {
        fseek(stdin, 0, SEEK_END);
        int flag;
        printf("1. Allocate memory\n2. Free memory\n");
        fgets(dataBuffer, BUFFER_SIZE, stdin);
        iResult = send(connectSocket, dataBuffer, BUFFER_SIZE, 0);
        if (iResult == SOCKET_ERROR) {
            printf("send failed with error: %d\n", WSAGetLastError());

        } else {
            printf("Bytes Sent: %d\n", iResult);
            flag = atoi(dataBuffer);
        }
      //  printf("Bytes Sent: %ld\n", iResult);
        fseek(stdin, 0, SEEK_END);
        fflush(stdin);

        iResult = recv(connectSocket, dataBuffer, BUFFER_SIZE, 0);

        if (iResult > 0) {
            if (flag == 1) {
                printf("Enter size of memory you want to allocate\n");
                fgets(dataBuffer, BUFFER_SIZE, stdin);
            } else if (flag == 2) {
                printf("Enter size of memory you want to free\n");
                fgets(dataBuffer, BUFFER_SIZE, stdin);
            }

            iResult = send(connectSocket, dataBuffer, BUFFER_SIZE, 0);

            if (iResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(connectSocket);
                WSACleanup();
                return 1;
            }
        } else {
            continue;
        }
        fseek(stdin, 0, SEEK_END);
        fflush(stdin);

        iResult = recv(connectSocket, dataBuffer, BUFFER_SIZE, 0);
        if (iResult > 0) {
            dataBuffer[iResult] = '\0';
            printf("Starting address of memory block is %s.\n", dataBuffer);
        } else {
            printf("received failed with error: %d\n", WSAGetLastError());
            closesocket(connectSocket);
            WSACleanup();
            return 1;
        }
    }
    // Close the socket
     iResult = shutdown(connectSocket, SD_BOTH);

	// Check if connection is succesfully shut down.
    if (iResult == SOCKET_ERROR)
    {
        printf("Shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

	// For demonstration purpose
	printf("\nPress any key to exit: ");
	getch();


    // Close connected socket
    closesocket(connectSocket);

	// Deinitialize WSA library
    WSACleanup();

	system("pause");

    return 0;

    return 0;
}
