#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <setjmp.h>
#include "update.h"

void doUpdate()
{
	// rsync is ideal for this situation as because it reduced the amount of data being sent by sending only the differences
	// between the source and already existing files
	// rsync command is used to pass the changes from the live folder to intranet
	char * update_exec_path = "rsync -r /var/www/html/Intranet/Bootstrap /var/www/html/Live";

	// run the rsync command and check return value
	if (system(update_exec_path) < 0)
	{	
		// failed to run command
		openlog("BackupDaemonLog", LOG_PID|LOG_CONS, LOG_USER);
		syslog(LOG_INFO, "Failed to update. Error Code: %s", strerror(errno));
		closelog();
	}
	else
	{
		// ran successfully
		openlog("BackupDaemonLog", LOG_PID|LOG_CONS, LOG_USER);
		syslog(LOG_INFO, "Website update.");
		closelog();
	}
}

