#!/bin/bash
for ((i=3;i<11;i+=1));do
	./ipc $i signals
	sleep 2
	./ipc $i pipe
	sleep 2
	./ipc $i lsock
	sleep 2
	./ipc $i socket
done
	
