#include <iostream>
#include <string>
#include <time.h>
#pragma warning(disable: 4996)

template <typename ...P> void Log(const P& ... strs) {
	std::stringstream stream;
	(stream << ... << strs);

	time_t rawTime;
	struct tm* timeInfo;
	char timeStr[80];

	time(&rawTime);
	timeInfo = localtime(&rawTime);

	strftime(timeStr, 80, "%I:%M:%S", timeInfo);

	std::cout << "[" << timeStr << "] " << stream.str() << std::endl;
}