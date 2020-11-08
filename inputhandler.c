#include <stdio.h>
#include <string.h>
#include <mqueue.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <syslog.h>

#define MAX_BUFFER 1024

typedef struct messagebuffer{
	long type;
	char text[MAX_BUFFER];
}msgbuf;

int main()
{
	// message queue id
	int msqid;
	// message queue flag
	int msgflag = IPC_CREAT | 0666;
	// unique message queue key
	key_t key = 1234;
	// message queue buffer
	struct messagebuffer sendbuffer;
	// message queue buffer length
	size_t bufferlength;
	
	// get the message queue id
	msqid = msgget(key, msgflag);
	// set the message queue type
	sendbuffer.type = 1;
	
	// ask the user for input until they ask to exit
	while(strncmp(sendbuffer.text, "exit", strlen("exit"))){
		// menu
		printf("\nChoice Daemon Option:");
		printf("\n---------------------");
		printf("\nbackup : Backup current live website");
		printf("\naudit : Log details of changes to Live");
		printf("\nterminate : Shutdown the running Daemon");
		printf("\nexit : Close the input handler\n");
		
		// read input
		scanf("%[^\n]",sendbuffer.text);
		getchar();
		
		// set the buffer to accomadate the input
		bufferlength = strlen(sendbuffer.text) + 1;
		
		// send the message and check if successful
		if(msgsnd(msqid, &sendbuffer, bufferlength, IPC_NOWAIT) < 0){
			// message failed to send
			printf("\nFailed to send message to Daemon. Please try again.");
			openlog("BackupDaemonLog", LOG_PID|LOG_CONS, LOG_USER);
			syslog(LOG_INFO, "Message Queue could not be reached.");
			closelog();	
		}
	}	
}




