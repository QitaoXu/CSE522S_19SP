<Team Members>
    Jiangnan Liu
    Qitao Xu
    Zhe Wang
</Team Members>

<Server Design>
    1. command line arguments handler
        Check number of command line arguments for each run and if this number does not equals to 3,
        server program will print a message such that "Usage: ./server <file name> <port number>" to 
        tell users how to run server program correctly.

    2. creating listening socket 
        According to the second argument in command line, server program can get to know the number
        of fragments reside in current directory, let's call this number as client_num, which also 
        indicates the maxium number of client can be accepted at runtime. After creating a listening 
        socket on port number given by last argument from command line, server program put listening
        socket into a struct pollfd array and use poll system call to realize multiplexed IO.

    3. data structure for each communication socket
        struct tracker{
            int skt;
            int is_sent;
            int is_recieved;
        };

        When a new client trying to connect server, poll system call can capture this activity, and 
        as long as number of built connection is not great than client_num, this client connecting 
        request can be accepted and poll system call also to monitor this socket on both POLLIN and 
        POLLOUT events.
        
        To track each communication socket, we design a specific data structure named tracker. There
        are three fields in this data structure: int skt, int is_sent, int is_recieved. For a new 
        clinet coming to connect, a tracker for this socket is initialiazed.

        When POLLOUT event is captured by on a communication socket on behalf of a client, server 
        program begins to read a fragment file and send each line to the client through this socket 
        until it reads to the endn of this fragment file. After that server program also sends a 
        send complete message to the client. And then server program stop monitoring POLLOUT event
        on this socket, set is_sent field of the tracker of this scoket to FINISHED and close this 
        fragment file.

    4. data structure for storing recieved sorted lines: AVL three
        When poll system call captures a POLLIN event on a communication socket, server program will 
        read messages sent from this socket and put it to AVL tree based on its line number. When
        server program recieves COMPLETE message from client, it will stop monitoring this socket,
        close this communication socket, is_recieved field of its tracker to FINISHED and increment 
        a flag, done, which is initialiazed as zero at the beginning. If all clients have sent data back, 
        done flag will equal to client_num, and server program will break the while loop for poll system
        call and write all lines in order to the output file. 

</Server Design>

<Client Design>
    1. command argument handler 
        Check number of command line arguments for each run and if this number does not equals to 3,
        client program will print a message such that 
        "Usage: ./client <IP address of server> <port number>\n" 
        to tell users how to run client program correctly.

    2. data structure to store and sort coming messages: AVL tree
        Taking advantage of select system call to monitor POLLIN events on socket connecting to
        server, client program repeatedly read messages when data is available from server after 
        connection, put line by line to AVL tree until it recieves COMPLTET. Then it will continue 
        to send messages in order back to server and exit.
</Client Design>

<Build Instructions>
    1. Make sure that all files in the same directory on 
       your raspberry pi.
    2. Open a command line and walk into directory holding
       all files.
    3. In the command line, issue "./compile.sh"
    4. Start server program, issue "./server jabberwocky 2000"
    5. If you want to run client program on the same machine 
       as server program, please issue "./local_run.sh"
    6. If you want to run client program on another machine,
       please run "./compile.sh" on that machine first, and
       according to the ip address printed out by server program,
       change the ip address in remote_run.sh, 
       and run "./remote_run.sh"
</Build Instructions>

<Testing and Evaluation>
    Case 1: Run server and client program on the same machine
            with jabberwocky
        Bugs: 1. It can work but there is one more newline in 
                line 15(line number starts from 0).
              2. Sometimes the output file cannot be opened with
                GUI directly but can be openned by vim. And there
                is a line ending with "^K". But sometimes it only
                triggers bug1.
    Case 2: Run server program on different machines with
            jabberwocky
        Bugs: 1. It can work but there is one more newline in 
                line 15(line number starts from 0).
</Testing and Evaluation>

<Development Effort>
    32h
</Development Effort>

