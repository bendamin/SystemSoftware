#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <syslog.h>
#include <setjmp.h>
#include <errno.h>
#include "date.h"

#define MAX_BUFFER 100

void doAudit()
{
	// declare variables
	// used for storing date info
	char datebuf[MAX_BUFFER];
	// get the date from date.c
	char * currentdate = getDate(datebuf);
	
	// set audit perameters for the ausearch and aureport commands to the intranet folder to be check and writen to auidits
	char * parameters = "ausearch -f /var/www/html/Intranet/ | aureport -ts today -te now -f -i > /home/ben/Documents/SystemSoftware/Audits/";
	// save the file as a text file
	char * extension = ".txt";
	// get the size of the buffer needed for the command
	int commandLength = strlen(parameters) + strlen(currentdate) + strlen(extension) + 1;
	// create a temporary buffer to store the command
	char * tempBuf = (char *) malloc (commandLength);
	// copy the command into the buffer
	strcpy(tempBuf, parameters);
	// add the date and extension to the string in the buffer to be used as a filename
	strcat(tempBuf, currentdate);
	strcat(tempBuf, extension);

	// check if the command ran successfully
	if (system(tempBuf) < 0)
	{
		// Error running the command
		openlog("BackupDaemonLog", LOG_PID|LOG_CONS, LOG_USER);
		syslog(LOG_INFO, "Audit Failed. Error Code: %s", strerror(errno));
		closelog();
	}
	else
	{
		// the command ran successfully
		openlog("BackupDaemonLog", LOG_PID|LOG_CONS, LOG_USER);
		syslog(LOG_INFO, "Audit Complete");
		closelog();
	}
	// free the temporary buffer
	free(tempBuf);
}
