#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/file.h>
#include <syslog.h>
#include <setjmp.h>
#include <fcntl.h> 
#include <signal.h> 
#include "audit.h"
#include "permissionmanager.h"
#include "backup.h"
#include "update.h"

#define MAX_BUFFER 1024
#define LOCKFILE "/var/run/BackupDaemon.pid"
#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define LOCK_SH 1

// struct for handling incoming message queue
typedef struct messagebuffer
{
	long type;
	char text[MAX_BUFFER];
}messagebuffer;

// signal handler
void sig_handler(int sigNum)
{
	// if SIGTERM is passed, such as when asked to terminate by the ManualInput program
	if (sigNum == SIGTERM){
		openlog("BackupDaemonLog", LOG_PID|LOG_CONS, LOG_USER);
		syslog(LOG_ERR, "Shutting Down Backup Daemon.");
		closelog();
		// reassign default handler
		signal(SIGTERM,SIG_DFL);
		exit(0);
	}else if (sigNum == SIGQUIT){
		// if SIGQUIT is passed such as when there is an error during the initial forks.
		openlog("BackupDaemonLog", LOG_PID|LOG_CONS, LOG_USER);
		syslog(LOG_ERR, "There was an unrecoverable error. Backup Daemon is shutting down.");
		closelog();
		// reassign default handler
		signal(SIGQUIT,SIG_DFL);
		exit(1);
	}
}

extern int lockfile(int);

int is_running(void)
{
	int fd;
	char buf[16];
	fd = open(LOCKFILE, O_RDWR|O_CREAT, LOCKMODE);
	if (fd < 0) 
	{
		openlog("BackupDaemonLog", LOG_PID|LOG_CONS, LOG_USER);
		syslog(LOG_ERR, "Cannot open the File Descriptor\n Error code: %s: %s", LOCKFILE, strerror(errno));
		closelog();
		exit(1);
	}
	if (flock(fd, LOCK_SH) < 0) 
	{
		if (errno == EACCES || errno == EAGAIN) 
		{
			close(fd);
			return(1);
		}
		
		openlog("BackupDaemonLog", LOG_PID|LOG_CONS, LOG_USER);
		syslog(LOG_ERR, "Lock failed with error code %s: %s", LOCKFILE, strerror(errno));
		closelog();
		exit(1);
	}
	ftruncate(fd, 0);
	sprintf(buf, "%ld", (long)getpid());
	write(fd, buf, strlen(buf)+1);
	return(0);
}

void main (int argc, char **argv) 
{
	// declare variables
	int x;
	int fd;
	int ppid;
	// Used to check clocktime for waiting on queue
	struct timespec timed;
	// time variables for checking current time
	time_t currenttime;
	struct tm timestruct;
	// message queue id	
	int msqid;
	// unique key for the message queue
	key_t key = 1234;
	// message queue buffer
	struct messagebuffer receivebuffer;
	
	// check for SIG_ERR during start up, mainly for child processes
	if(signal(SIGTERM, sig_handler) == SIG_ERR){
		printf("An Error Occured");
	}
	
	// start audit
	char * audit = "auditctl -w /var/www/html/Intranet/Bootstrap -p rwxa";
	// if audit could not be started successfully
	if (system(audit) < 0)
	{
		openlog("BackupDaemonLog", LOG_PID|LOG_CONS, LOG_USER);
		syslog(LOG_INFO, "Audit could not be started. Error Code: %s", strerror(errno));
		closelog();
	} 
	else 
	{
		// audit has been started
		openlog("BackupDaemonLog", LOG_PID|LOG_CONS, LOG_USER);
		syslog(LOG_INFO, "Audit is running");
		closelog();
	}

	// fork to allow background daemon
	int pid = fork();
	// if the fork was unsuccessful
	if (pid < 0)
	{
		ppid = getpid();
		openlog("BackupDaemonLog", LOG_PID|LOG_CONS, LOG_USER);
		syslog(LOG_INFO, "Fork failed. Error Code: %s", strerror(errno));
		closelog();
		// kill the process using the signal handler
		kill(ppid, SIGQUIT);
	}
	
	
	// kill parent to create orphan
	if (pid > 0)
	{
		printf("\nBackup Daemon has started.\n");
		sleep(1);
		exit(EXIT_SUCCESS);
	} else if(pid == 0){
		// the child
		
		// set the session id and check if it was unsuccessful
		if(setsid() < 0){
			openlog("BackupDaemonLog", LOG_PID|LOG_CONS, LOG_USER);
			syslog(LOG_INFO, "Setting Session ID Failed. Error Code: %s", strerror(errno));
			closelog();
			// kill the process using the signal handler
			kill(ppid, SIGQUIT);
		}
		// fork and store the new pid
		pid = fork();
    
		// if the fork failed, log and close the program
		if (pid < 0)
		{
			ppid = getpid();
			openlog("BackupDaemonLog", LOG_PID|LOG_CONS, LOG_USER);
			syslog(LOG_INFO, "Failed to fork with error code: %s", strerror(errno));
			closelog();
			kill(ppid, SIGQUIT);
		}
		
		// Terminate the parent
		if (pid > 0)
		{
			sleep(1);
			exit(EXIT_SUCCESS);
		}	
		umask(0);
		// change directory
		if (chdir("/") < 0) 
		{
			// failed to change directory
			openlog("BackupDaemonLog", LOG_PID|LOG_CONS, LOG_USER);
			syslog(LOG_INFO, "Directory Change Failed. Error Code: %s", strerror(errno));
			closelog();
			// kill the process
			kill(ppid, SIGQUIT);
		}
		// ensure all file descriptors are closed
		int maxfiledescriptor = sysconf(_SC_OPEN_MAX);
		for(x = 3; x < maxfiledescriptor; x++)
		{
			close(x);
		}
		
		fd = open("/dev/null", O_RDWR);
            	dup2(fd, STDIN_FILENO); 
            	dup2(fd, STDOUT_FILENO); 
           	dup2(fd, STDERR_FILENO);
		
		// the while loop of the daemon
		while(1) 
		{	
			// if the queue id was set
			if((msqid = msgget(key, 0666)) > -1){
				// if the queue received a message
				if(msgrcv(msqid, &receivebuffer, MAX_BUFFER,1, MSG_NOERROR | IPC_NOWAIT) > -1){
					// if backup was passed
					if(! strncmp(receivebuffer.text, "backup", strlen("backup")))
					{
						// change file permissions
						setPermissions("1111");
						// backup
						doBackup();
						sleep(6);
						// reset permissions
						setPermissions("0777");
					}
					// if update was passed
					else if(! strncmp(receivebuffer.text, "update", strlen("update")))
					{
						// change file permissions
						setPermissions("1111");
						// update
						doUpdate();
						sleep(6);
						// reset permissions
						setPermissions("0777");
					}
					// if terminate was passed
					else if(! strncmp(receivebuffer.text, "terminate", strlen("terminate")))
					{
						ppid = getpid();
						kill(ppid, SIGTERM);
					}
					openlog("BackupDaemonLog", LOG_PID|LOG_CONS, LOG_USER);
					syslog(LOG_INFO, "InputHandled: %s", receivebuffer.text);
					closelog();
					
				}
			}
			
			openlog("BackupDaemonLog", LOG_PID|LOG_CONS, LOG_USER);
				syslog(LOG_INFO, "NoInputHandled: %s", receivebuffer.text);
				closelog();
						
			// get current localtime
			currenttime = time(NULL);
			timestruct = *localtime(&currenttime);
			
			// check if it is time for automatic backup
			if((timestruct.tm_hour == 14) && (timestruct.tm_min == 26))
			{
				//change file permissions
				setPermissions("1111");
				//audit
				doAudit();
				//backup
				doBackup();
				//update
				doUpdate();
				sleep(6);
				// reset permissions
				setPermissions("0777");
				// sleep as only one backup needed this minute
				sleep(55);

			}
		}
 	}
}
