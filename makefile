CC = gcc
objects = main.o date.o audit.o permissionmanager.o backup.o update.o 
cfiles =  main.c date.c audit.c permissionmanager.c backup.c update.c 
headers = date.h audit.h permissionmanager.h backup.h update.h
name = BackupDaemon

BackupDaemon : $(objects)
	$(CC) -o $(name) $(objects) -lm -lrt
	$(CC) inputhandler.c -o ManualInput -lrt

main.o : main.c $(headers)
	$(CC) -c main.c 

date.o : date.c
	$(CC) -c date.c

audit.o : audit.c
	$(CC) -c audit.c
	
filepermissions.o : permissionmanager.c
	$(CC) -c permissionmanager.c
	
backup.o : backup.c
	$(CC) -c backup.c
	
update.o : update.c
	$(CC) -c update.c
	
inputhandler.o: inputhandler.c
	$(CC) -c inputhandler.c

debug : $(cfiles)
	$(CC) -g $(cfiles) -lm -o $(name)

clean:
	rm $(name) $(objects)
