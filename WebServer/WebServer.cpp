// WebServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "Server.h"
#include <iostream>

int main()
{
	Server srv = Server((char*)"8080");
	srv.ServerListen();
    std::cout << "Exiting!\n"; 
}
