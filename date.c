#include <stdio.h>
#include <time.h>

#define MAX_BUFFER 80

char* getDate(char * buffer)
{
	// variables for storing time info
	time_t currenttime;
	struct tm * timeinfo;
	// set the time in currenttime
	time (&currenttime);
	// retreive the localtime
	timeinfo = localtime(&currenttime);
	// formate the date into a string to be returned
	strftime(buffer,MAX_BUFFER,"%Y-%m-%d-%H-%M", timeinfo);
	// return
	return buffer;
}
