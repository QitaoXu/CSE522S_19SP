#!/bin/bash
for ((i=3;i<11;i+=1));do
<<<<<<< HEAD
	./ipc $i signals >> signals.txt
	sleep 2
	./ipc $i pipe >> pip.txt
	sleep 2
	./ipc $i lsock >> lsock.txt
	sleep 2
	./ipc $i socket >>socket.txt
	sleep 2
=======
	./ipc $i signals
	sleep 2
	./ipc $i pipe
	sleep 2
	./ipc $i lsock
	sleep 2
	./ipc $i socket
>>>>>>> ce4d98572ade2e498794de7da086666d7a5c4c4b
done
	
