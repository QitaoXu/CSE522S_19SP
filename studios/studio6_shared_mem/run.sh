#!/bin/bash
for ((i=3;i<11;i+=1));do
	./ipc $i signals >> signals.txt
	sleep 2
	./ipc $i pipe >> pip.txt
	sleep 2
	./ipc $i lsock >> lsock.txt
	sleep 2
	./ipc $i socket >>socket.txt
	sleep 2
done
	
