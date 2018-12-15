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

int Server::ErrorHandler(int iRes, SOCKET * sock, addrinfo * addr)
{
	return 0;
}

int Server::ServerListen()
{
	WSADATA wsaData;
	int iResult, iSendResult, recvbuflen = DEFAULT_BUFLEN;
	SOCKET ListenSocket = INVALID_SOCKET, ClientSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL, hints;
	char recvbuf[DEFAULT_BUFLEN];

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
	closesocket(ListenSocket);

	iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
	if (iResult > 0) {
		printf("Bytes received: %d\n", iResult);
		std::vector<std::string> splits;
		Utils::SearchString(recvbuf, &splits, ' ');
		printf_s("Requested File: %s\n", splits[1].substr(1, splits[1].length()).c_str());
		char* fileFound;
		fileFound = Server::searchDirectory((char*)splits[1].substr(1, splits[1].length()).c_str());
		char* file;
		char*  hey;
		if (fileFound != "")
		{
			file = Server::fileConsume((char*)splits[1].substr(1, splits[1].length()).c_str());
			hey = (char*)"HTTP / 1.1 200 OK\nServer: DeltaX / 1.0.0\nContent - Type: text / html\n\n";
		}
		else 
		{
			file = Server::fileConsume((char*)"404.html");
			hey = (char*)"HTTP / 1.1 404 OK\nServer: DeltaX / 1.0.0\nContent - Type: text / html\n\n";
		}
		// Send over the correct file
		std::string finalWord(hey);
		std::string f2(file);
		finalWord += f2;
		iSendResult = send(ClientSocket, finalWord.c_str(), finalWord.length(), 0);
		shutdown(ClientSocket, SD_SEND);
		closesocket(ClientSocket);
		WSACleanup();
		return 0;
			 
		if (iSendResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}
		printf("Bytes sent: %d\n", iSendResult);
	}
	else if (iResult == 0)
		printf("Connection closing...\n");
	else {
		printf("recv failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	return 0;
}

char* Server::fileConsume(char* fileName)
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
		printf(TEXT("Terminal failure: unable to open file \"%s\" for read.\n"), fileName);
		return (char*)"Error";
	}

	// Get File Size
	GetFileSizeEx(hFile, &fileSize);

	printf_s("File Size: %lld\n", fileSize.QuadPart);

	// Read one character less than the buffer size to save room for
	// the terminating NULL character. 

	if (FALSE == ReadFile(hFile, ReadBuffer, (DWORD)fileSize.QuadPart, NULL, NULL))
	{
		printf("Terminal failure: Unable to read from file.\n GetLastError=%08x\n", GetLastError());
		CloseHandle(hFile);
		return (char*)"Error";
	}

	// It is always good practice to close the open file handles even though
	// the app will exit here and clean up open handles anyway.

	CloseHandle(hFile);
	return ReadBuffer;
}

char* Server::searchDirectory(char* fileName)
{
	static WIN32_FIND_DATAA searchData;

	FindFirstFileA(fileName, &searchData);

	if (searchData.cFileName != fileName) {
		// File not found
		return (char*)"";
	}

	printf_s("Found File Name: %s\n", searchData.cFileName);
	return searchData.cFileName;
}