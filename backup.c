#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <setjmp.h>
#include "date.h"
#include "backup.h"

#define MAX_BUFFER 80

void doBackup()
{
	// buffer for storing the date
	char datebuf [MAX_BUFFER];
	// get the date from date.c
	char * currentdate = getDate(datebuf);
	// set the parameters of the command to copy the fold from live to backups
	char * parametrers = "cp -R /var/www/html/Live /home/ben/Documents/SystemSoftware/Backups/";
	// set the extension
	char * extension = ".zip";
	// get the length of the command
	int commandLength = strlen(parametrers) + strlen(currentdate) +  strlen(extension) + 1;
	// buffer for storing the final command
	char * tempBuf = (char *)malloc(commandLength);
	// copy the command into the command buffer
	strcpy(tempBuf, parametrers);
	// add the date and extension as the file name to the command
	strcat(tempBuf, currentdate);
	strcat(tempBuf, extension);

	// if the command failed to execute
	if (system(tempBuf) < 0)
	{
		openlog("BackupDaemonLog", LOG_PID|LOG_CONS, LOG_USER);
		syslog(LOG_INFO, "Backup Failed. Error Code: %s", strerror(errno));
		closelog();
	}
	else
	{
		// ran successfully
		openlog("BackupDaemonLog", LOG_PID|LOG_CONS, LOG_USER);
		syslog(LOG_INFO, "Backup Complete.");
		closelog();
	}
	
	free(tempBuf);
}
