#include "pch.h"
#include "Utils.h"

void Utils::SearchString(char* inBuffer, std::vector<std::string> *strings, char delimiter, int startIndex)
{
	char outBuffer[DEFAULT_BUFLEN];
	memset(outBuffer, 0x00, DEFAULT_BUFLEN);
	int outBufferInt = 0, startInt = 0;
	if (startIndex) startInt = startIndex;
	for (size_t i = startInt; i < strlen(inBuffer); i++)
	{
		if (inBuffer[i] == delimiter) {
			// Flush string to vector
			strings->emplace_back(outBuffer);
			// Call this again with new location
			SearchString(inBuffer, strings, delimiter, i + 1);
			return;
		}
		// Copy first part of string
		outBuffer[outBufferInt] = inBuffer[i];
		outBufferInt++;
	}
	// Flush last string in
	strings->emplace_back(outBuffer);
}