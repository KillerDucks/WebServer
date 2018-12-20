#include "pch.h"
#include "Server.h"


Server::Server(char* serverPort)
{
	// Pass the variable through
	Server::serverPort = serverPort;
}


Server::~Server()
{
}

int Server::ErrorHandler(int iRes, char* error, SOCKET * sock, addrinfo * addr)
{
	switch (iRes)
	{
	case !0:
		printf_s("%s failed with error: %d\n", error, iRes);
		if (error != (char*)"WSAStartup") WSACleanup();
		return 1;
	case 0:
		if (sock != NULL) {
			printf_s("%d failed with error: %ld\n", error, WSAGetLastError());
			if (addr) {
				freeaddrinfo(addr);
			}
			else {
				closesocket(*sock);
			}
			return 1;
		}
		return 0;
	default:
		return 0;
	}
	return 0;
}

int Server::ServerListen()
{
	WSADATA wsaData;
	int iResult, recvbuflen = DEFAULT_BUFLEN;
	SOCKET ListenSocket = INVALID_SOCKET, ClientSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL, hints;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, Server::serverPort, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	do
	{
		iResult = listen(ListenSocket, SOMAXCONN);
		if (iResult == SOCKET_ERROR) {
			printf("listen failed with error: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		// Accept a client socket
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed with error: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		// No longer need server socket
			//closesocket(ListenSocket);

		// Parse Request -> Serve Data
		if (Server::RequestParse(ClientSocket))
		{
			printf("[Server::ServerListen]\tFailed to Parse Client Request\n");
		}

	} while (true);
	
	WSACleanup();

	return 0;
}

int Server::RequestParse(SOCKET clientSock)
{
	int		iResult, inBuffLen = DEFAULT_BUFLEN;
	char	inBuff[DEFAULT_BUFLEN];
	
	iResult = recv(clientSock, inBuff, inBuffLen, 0);

	if (iResult > 0) {
		char*						file;
		char*						header;
		std::vector<std::string>	splits;

		printf("[Server::RequestParse]\tBytes received: %d\n", iResult);
		
		Utils::SearchString(inBuff, &splits, ' ');
		printf_s("[Server::RequestParse]\tRequested File: %s\n", splits[1].substr(1, splits[1].length()).c_str());
		
		if (Server::SearchDirectory((char*)splits[1].substr(1, splits[1].length()).c_str()))
		{
			file = Server::FileConsume((char*)splits[1].substr(1, splits[1].length()).c_str());
			header = (char*)"HTTP / 1.1 200 OK\nServer: DeltaX / 1.0.0\nContent - Type: text / html\n\n";
		}
		else
		{
			file = Server::FileConsume((char*)"404.html");
			header = (char*)"HTTP / 1.1 404 Not Found\nServer: DeltaX / 1.0.0\nContent - Type: text / html\n\n";
		}

		// Send over the correct file
		std::string httpResponse(header), requestedData(file);
		httpResponse += requestedData;

		if (Server::SendBuffer(clientSock, (char*)httpResponse.c_str(), (int)httpResponse.length()))
		{
			closesocket(clientSock);
		}

		shutdown(clientSock, SD_SEND);
		closesocket(clientSock);
		
		return 0;
		
	}
	else {
		printf("[Server::RequestParse]\trecv failed with error: %d\n", WSAGetLastError());
		closesocket(clientSock);
		WSACleanup();
		return 1;
	}
}

int Server::SendBuffer(SOCKET clientSock, char* outBuffer, int dataLength)
{
	int		iSendResult = 0;

	iSendResult = send(clientSock, outBuffer, dataLength, 0);

	// Check for Socket Errors
	if (iSendResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		return 1;
	}

	printf("[Server::SendBuffer]\tBytes sent: %d\n", iSendResult);

	return 0;
}

char* Server::FileConsume(char* fileName)
{
	#define BUFFERSIZE 1024
    HANDLE hFile;
	DWORD  dwBytesRead = 0;
	static char   ReadBuffer[BUFFERSIZE] = { 0 };
	LARGE_INTEGER fileSize;

	hFile = CreateFile(	fileName,               // file to open
						GENERIC_READ,          // open for reading
						FILE_SHARE_READ,       // share for reading
						NULL,                  // default security
						OPEN_EXISTING,         // existing file only
						FILE_ATTRIBUTE_NORMAL | FILE_READ_ATTRIBUTES, // normal file
						NULL);                 // no attr. template

	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("[Server::FileConsume]\tTerminal failure: unable to open file \"%s\" for read.\n", fileName);
		return (char*)"Error";
	}

	// Get File Size
	GetFileSizeEx(hFile, &fileSize);

	printf_s("[Server::FileConsume]\tFile Size: %lld\n", fileSize.QuadPart);

	// Read one character less than the buffer size to save room for
	// the terminating NULL character. 

	if (FALSE == ReadFile(hFile, ReadBuffer, (DWORD)fileSize.QuadPart, NULL, NULL))
	{
		printf("[Server::FileConsume]\tTerminal failure: Unable to read from file.\n GetLastError=%08x\n", GetLastError());
		CloseHandle(hFile);
		return (char*)"Error";
	}

	// It is always good practice to close the open file handles even though
	// the app will exit here and clean up open handles anyway.

	CloseHandle(hFile);
	return ReadBuffer;
}

bool Server::SearchDirectory(char* fileName)
{
	static WIN32_FIND_DATAA searchData;

	FindFirstFileA(fileName, &searchData);

	if (searchData.nFileSizeLow == 0) {
		// File not found
		printf_s("[Server::SearchDirectory]\tUnable to Find File Name: %s\n", fileName);
		return false;
	}

	printf_s("[Server::SearchDirectory]\tFound File Name: %s\n", searchData.cFileName);
	return true;
}