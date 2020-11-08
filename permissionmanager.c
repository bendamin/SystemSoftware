#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <syslog.h>
#include <setjmp.h>

#define MAX_BUFFER 200

void setPermissions(char mode[])
{
	// declare variables
	int x;
	struct stat tempStat;
	
	// set the path
	char path[MAX_BUFFER] = "/var/www/html";
	// retrieve information on the path
	stat(path, &tempStat);
	
	// convert to long int
	x = strtol(mode, 0, 8);
	
	// check if permissions set
	if(chmod(path, x) < 0)
	{
		// invalid permissions
		openlog("BackupDaemonLog", LOG_PID|LOG_CONS, LOG_USER);
    		syslog(LOG_INFO, "Permissions Not Set. Error Code: %s", strerror(errno));
    		closelog();
	}
	else
	{
		// permissions set
		openlog("BackupDaemonLog", LOG_PID|LOG_CONS, LOG_USER);
    		syslog(LOG_INFO, "Permissions Set.");
    		closelog();
	}
}
