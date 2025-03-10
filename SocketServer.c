#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>

struct ClientSocket
{
    int client_socket_fd;
    struct sockaddr_in clientAddress;
    socklen_t clientAddressSize;
    bool createdSuccessfully;
};

struct ClientSocket* listOfConnectedClientSockets[10];

int connectedClientSocketsCount = 0;

void sendReceivedMessageToOtherClients(char messageFromClient[] , int new_client_socket_fd)
{
    // printf("Message to be send to other clients: \n");
    // printf("%s\n\n" , messageFromClient);
    send(new_client_socket_fd , messageFromClient , strlen(messageFromClient) , 0);
}

void receiveMessagesFromClient(int new_client_socket_fd)
{
    char messageFromClient[1024];

    while(true)
    {
        // recv() is a blocking call
        int numOfBytesRead = recv(new_client_socket_fd , messageFromClient , sizeof(messageFromClient) , 0);

        printf("Number of Bytes read with client socket fd: %d = %d bytes\n" , new_client_socket_fd , numOfBytesRead);

        if(numOfBytesRead == 0)
        {
            printf("Client with client_socket_fd: %d Disconnected !\n\n" , new_client_socket_fd);
            break;
        }
        else if(numOfBytesRead < 0)
        {
            printf("Error recieving data from client !\n\n");
            break;
        }

        messageFromClient[numOfBytesRead] = '\0';

        // Displaying the message recieved from client
        printf("Message recieved from Client with client socket fd: %d\n" , new_client_socket_fd);
        printf("%s\n\n" , messageFromClient);

        for(int i = 0 ; i < connectedClientSocketsCount ; i++)
        {
            if(listOfConnectedClientSockets[i] -> client_socket_fd != new_client_socket_fd)
            {
                sendReceivedMessageToOtherClients(messageFromClient , listOfConnectedClientSockets[i] -> client_socket_fd);
            }
        }
    }

    close(new_client_socket_fd);

    return;
}

void startReceivingMessagesOnSeparateThread(int new_client_socket_fd)
{
    pthread_t tid;

    pthread_create(&tid , NULL , receiveMessagesFromClient , new_client_socket_fd);
}

struct ClientSocket* createClientSocketForIncomingConnection(int serverSocketFd)
{
    struct ClientSocket* clientSocket = malloc( sizeof(struct ClientSocket) );

    clientSocket -> clientAddressSize = sizeof(struct sockaddr_in);

    clientSocket -> client_socket_fd = accept(serverSocketFd , (struct sockaddr*)(&clientSocket -> clientAddress) , &(clientSocket -> clientAddressSize) );

    if(clientSocket -> client_socket_fd == -1)
    {
        printf("Failed to connect to client !\n");
        clientSocket -> createdSuccessfully = false;
        free(clientSocket);
        return NULL;
    }

    clientSocket -> createdSuccessfully = true;

    printf("Server connected to client successfully\n");

    printf("New Client Socket File Descriptor: %d\n\n" , clientSocket -> client_socket_fd);

    return clientSocket;

}

void startAcceptingIncomingConnections(int serverSocketFd)
{
    while(true)
    {
        struct ClientSocket* new_client_socket = createClientSocketForIncomingConnection(serverSocketFd);
        if(new_client_socket -> createdSuccessfully == true)
        {
            listOfConnectedClientSockets[connectedClientSocketsCount++] = new_client_socket;
            startReceivingMessagesOnSeparateThread(new_client_socket -> client_socket_fd);
        }
    }
}

int main()
{
    // Creating a socket on the server
    int serverSocketFd = socket(AF_INET , SOCK_STREAM , 0);

    if(serverSocketFd == -1)
    {
        printf("Failed to create socket\n");
        return 1;
    }

    printf("Server Socket created successfully....\n");

    printf("Server Socket File Descriptor: %d\n" , serverSocketFd);

    struct sockaddr_in address;

    address.sin_port = htons(2000);  // PORT on which the server will listen
    address.sin_family = AF_INET;    // Address family for IPv4
    address.sin_addr.s_addr = INADDR_ANY;  // Address  to accept any incoming connections/requests

    // NOTE: In case of server, the address is not of some "client" that the server wants to 
    // connect to. 

    // Binding serverSocketFD to the local address "address" created above
    int bindFeedback = bind(serverSocketFd , (struct sockaddr*) &address , sizeof(address));

    if(bindFeedback == -1)
    {
        printf("Could not bind socket to local address\n");
        return 2;
    }

    if(bindFeedback == 0)
    {
        printf("Socket bound successfully....\n");
    }

    // listen for connections on a socket.
    // Prepare to accept connections on socket FD.
    // N connection requests will be queued before further requests are refused.
    // listen() marks the socket referred to by sockfd as a passive socket, that is, 
    // as a socket that will be used to accept incoming connection requests using accept().

    int listenFeedback = listen(serverSocketFd , 10);

    if(listenFeedback == -1)
    {
        printf("Could not listen to incoming connections\n");
        return 3;
    }

    if(listenFeedback == 0)
    {
        printf("Listening to incoming connections....\n\n");
    }

    // accept(): extracts the first connection request on the queue of pending connections 
    // for the listening socket, serverSocketFd, creates a new connected socket, and returns a 
    // new file descriptor referring to that socket. 

    // The newly created socket is not in the listening state.

    // The original socket serverSocketFd is unaffected by this call.

    // The  argument  sockfd is a socket that has been created with socket(), bound to a 
    // local address with bind(), and is listening for connections after a listen().

    // Await a connection on serverSocketFd.
    // When a connection arrives, open a new socket to communicate with it, 
    // set *ADDR (which is *ADDR_LEN bytes long) to the address of the connecting peer and 
    // *ADDR_LEN to the address's actual length, and return the new socket's descriptor, 
    // or -1 for errors.

    startAcceptingIncomingConnections(serverSocketFd);

    int shutdown_status = shutdown(serverSocketFd , SHUT_RDWR);

    if(shutdown_status == -1)
    {
        printf("Server Socket failed to shutdown !\n");
        return 4;
    }

    printf("Server Socket shutdown successfully !\n");

    return 0;
}