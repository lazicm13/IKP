#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>  // Add this header for bool
#include <winsock2.h>
#include <unistd.h>  // Add this header for close

#define SERVER_PORT 4010
#define BUFFER_SIZE 1024

HANDLE semaphore1;

void* _alloc(int size) {
	WaitForSingleObject(semaphore1, INFINITE);
	void* ret = malloc(size);
	ReleaseSemaphore(semaphore1, 1, NULL);
	return ret;
}

void _free(void* address) {
	WaitForSingleObject(semaphore1, INFINITE);
	free(address);
	ReleaseSemaphore(semaphore1, 1, NULL);
}


int main() {
    SOCKET listenSocket = INVALID_SOCKET;

	// Socket used for communication with client
	SOCKET acceptedSocket;

	// Variable used to store function return value
	int iResult, iResultSend;

	// Buffer used for storing incoming data
	char dataBuffer[BUFFER_SIZE];

	// WSADATA data structure that is to receive details of the Windows Sockets implementation
	WSADATA wsaData;

	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return 1;
	}


	semaphore1 = CreateSemaphore(NULL, 1, 1, NULL);
    if (semaphore1 == NULL) {
        printf("Failed to create semaphore: %d\n", GetLastError());
        return 1;
    }


	// Initialize serverAddress structure used by bind
    struct sockaddr_in serverAddress;
	memset((char*)&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;				// IPv4 address family
	serverAddress.sin_addr.s_addr = INADDR_ANY;		// Use all available addresses
	serverAddress.sin_port = htons(SERVER_PORT);	// Use specific port


	// Create a SOCKET for connecting to server
	listenSocket = socket(AF_INET,      // IPv4 address family
		SOCK_STREAM,  // Stream socket
		IPPROTO_TCP); // TCP protocol

	// Check if socket is successfully created
	if (listenSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket - bind port number and local address to socket
	iResult = bind(listenSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress));

	// Check if socket is successfully binded to address and port from sockaddr_in structure
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	//Postavljanje soketa u neblokirajuci rezim
	unsigned long mode = 1; //non-blocking mode
	iResult = ioctlsocket(listenSocket, FIONBIO, &mode);
	if (iResult != NO_ERROR) {
		printf("ioctlsocket failed with error: %d\n", iResult);
	}

	// Set listenSocket in listening mode
	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	printf("Server socket is set to listening mode. Waiting for new connection requests.\n");

    fd_set readSet;
    struct timeval timeout;


	bool allocFlag;
	do
	{
	    FD_ZERO(&readSet);
        FD_SET(listenSocket, &readSet);

        iResult = select(listenSocket + 1, &readSet, NULL, NULL, &timeout);

		if (iResult == -1)
		{
			fprintf(stderr, "select failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}
		else if (iResult > 0) {
			acceptedSocket = accept(listenSocket, NULL, NULL);

			if (acceptedSocket == INVALID_SOCKET)
			{
				printf("accept failed with error: %d\n", WSAGetLastError());
				closesocket(acceptedSocket);
				WSACleanup();
				return 1;
			}

			//non blocking
			mode = 1;
			iResult = ioctlsocket(acceptedSocket, FIONBIO, &mode);
			if (iResult != NO_ERROR)
				printf("ioctlsocket failed with error: %d\n", iResult);

		}
		else
		{
                /// ODAVDE NEGDE DOLE PUKNE, NA RECV VRV
				iResult = recv(acceptedSocket, dataBuffer, BUFFER_SIZE, 0);
				printf("RECV: %d", iResult);
				if (iResult > 0)
				{
					int size = atoi(dataBuffer);

					if (size <= 2 && size > 0) {
						if (size == 1) {
							allocFlag = true;
						}
						else if (size == 2) {
							allocFlag = false;
						}

						memset(dataBuffer, 0, BUFFER_SIZE);
						strcpy_s(dataBuffer, sizeof(dataBuffer), "ACK");
						iResultSend = send(acceptedSocket, dataBuffer, (int)strlen(dataBuffer), 0);

					}
					else if (size > 0) {
						if (allocFlag) {
							printf("Client is requesting %d bytes allocated.\n", size);
							unsigned int ret = (unsigned int)_alloc(size);
							itoa(ret, dataBuffer, 10);
						}
						else {
							_free((void*)size);
							printf("Client is freeing %d allocated address.\n", size);
							itoa(size, dataBuffer, 10);
						}
						iResultSend = send(acceptedSocket, dataBuffer, (int)strlen(dataBuffer), 0);
						//_print(masterHeapManager);
					}
				}
				else if (iResult == 0)
				{
					printf("Connection with client closed.\n");
					closesocket(acceptedSocket);
				}
				else if(iResult == -1)
				{

					printf("recv failed with error: %d\n", WSAGetLastError());
					closesocket(acceptedSocket);
				}
				else {
					continue;
				}

		}

	} while (strcmp(dataBuffer, "poyy") != 0);

		iResult = shutdown(acceptedSocket, SD_BOTH);

		if (iResult == SOCKET_ERROR)
		{
			printf("shutdown failed with error: %d\n", WSAGetLastError());
			closesocket(acceptedSocket);
			WSACleanup();
			return 1;
		}
		closesocket(acceptedSocket);


	//Close listen and accepted sockets
	closesocket(listenSocket);

	printf("graceful shutdown!\n");
	// Deinitialize WSA library
	WSACleanup();

	return 0;
}
