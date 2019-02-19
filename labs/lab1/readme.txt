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
    2. data structure to store and sort coming messages: AVL tree
</Client Design>

<Build Instructions>
</Build Instructions>

<Testing and Evaluation>
</Testing and Evaluation>

<Development Effort>
    32h
</Development Effort>

