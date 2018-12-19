#pragma once

#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#include	<cstdio>
#include	<vector>
#include	<iostream>

#include	"Utils.h"

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

class Server
{
public:
	// Structs
	typedef struct HTTPHeader {
		// Std HTTP headers here
	} HTTPHEADER, PHTTPHEADER;

	typedef struct HTTPMessage {

	} HTTPMESSAGE, PHTTPMESSAGE;

	typedef struct HTTPObject {
		PHTTPHEADER		pHTTPHeader;
		PHTTPMESSAGE	pHTTPMessage;
	} HTTPOBJECT, PHTTPOBJECT;

	// Variables
	char* serverPort;

	// Constructors / Destructors
	Server(char* serverPort);
	~Server();

	// Error Handling
	int			ErrorHandler(int iRes, char* error, SOCKET *sock = NULL, addrinfo *addr = NULL);														// Handle any errors from the WinSock server

	// Data parsing
	void		RemovePadding(char* inBuffer, int dataLength, char* outBuffer);		// Remove the random data from the incomming buffer
	HTTPObject	GetHTTPMeta(char* inBuffer); 										// A way to strip the HTTP header from the incoming data
	char*		CreateBuffer(HTTPObject httpObject);

	// Data Transmission
	int			SendBuffer(char* outBuffer, int dataLength);						// Send a completed buffer back to the client

	// Server
	int			ServerListen();

	// File I/O
	static char*		fileConsume(char* fileName);
	static char*		searchDirectory(char* fileName);
};

