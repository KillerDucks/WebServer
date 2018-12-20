// WebServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "Server.h"
#include <iostream>

int main(int argc, char* argv[])
{
	if (argc < 1)
	{
		if (strlen((char*)argv[1]) == 4)
		{
			if (isdigit(argv[1][0]))
			{
				Server srv = Server((char*)argv[1]);
				srv.ServerListen();

				std::cout << "Exiting!\n";

				return 1;
			}
		}
	}
		
		
	Server srv = Server((char*)"8080");
	srv.ServerListen();

    std::cout << "Exiting!\n"; 

	return 1;
}
