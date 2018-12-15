#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string.h>
#include <string>

#define DEFAULT_BUFLEN 512

class Utils
{
public:
	void static SearchString(char* inBuffer, std::vector<std::string> *strings, char delimiter, int startIndex = 0);
};

