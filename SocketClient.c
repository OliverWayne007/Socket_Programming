#include <stdio.h>

#include <sys/types.h>

#include <sys/socket.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <string.h>

#include <stdbool.h>

#include <pthread.h>

void receiveMessagesFromServer(int socketFd)
{
    char messageReceivedFromServer[1024];

    while(true)
    {
        int numOfBytesRead = recv(socketFd , messageReceivedFromServer , sizeof(messageReceivedFromServer) , 0);

        if(numOfBytesRead > 0)
        {
            messageReceivedFromServer[numOfBytesRead] = '\0';
            // printf("Message received from server: \n");
            printf("%s\n" , messageReceivedFromServer);
        }
        else if(numOfBytesRead == 0)
        {
            printf("Disconnected from server !\n");
            break;
        }
        else
        {
            printf("Error receiving messages from server !\n");
            break;
        }
    }

    return;
}

void startReceivingMessagesFromTheServer(int socketFd)
{
    pthread_t tid;
    pthread_create(&tid , NULL , receiveMessagesFromServer , socketFd);
}

void sendMessagesToTheServer(int socketFd)
{
    char message[1024];

    while(true)
    {
        // printf("Enter the message to send to server: \n");
        fgets(message , sizeof(message) , stdin);

        // Removing the newline character '\n' from fgets
        message[strcspn(message , "\n")] = '\0';

        if(strcmp(message , "exit") == 0)
        {
            break;
        }

        // printf("Message to be send to the server: \n");
        // printf("%s\n" , message);
        // printf("Length of message to be send to the server: %zu\n\n" , strlen(message));

        send(socketFd , message , strlen(message) , 0);
    }

    return;
}

int main()
{
    // socket()  creates  an  endpoint  for communication and returns a file descriptor that 
    // refers to that endpoint.

    // The file descriptor returned by a successful call will be the lowest-numbered file 
    // descriptor  not  currently open for the process.

    // The  domain argument specifies a communication domain; this selects the protocol family 
    // which will be used for communication.  These families are defined in <sys/socket.h>.


    // Creating a Socket
    // int socketFd = socket(domain , type , protocol);

    int socketFd = socket(AF_INET , SOCK_STREAM , 0);

    // Internet domain: AF_INET ---> Address Family for IPv4 ---> Inter-Machine
    // Internet domain: AF_INET6 ---> Address Family for IPv6 ---> Inter-Machine
    // Unix Domain: AF_UNIX ---> Address Family for Unix-File system ---> Intra-Machine

    // AF_INET and AF_UNIX sockets can be connection-oriented (SOCK_STREAM)
    // or connectionless (SOCK_DGRAM) or (SOCK_RAW)

    // Connection-Oriented AF_INET sockets use Transmission-Control-Protocol (TCP) and
    // Connectionless AF_INET sockets use User-Datagram-Protocol (UDP).

    if(socketFd == -1)
    {
        printf("Failed to create Socket\n");
        return 1;
    }

    printf("Client Socket File Descriptor: %d\n" , socketFd);

    // ----------------------------------------------------------------------------------

    // sockaddr is like the base:

    //    struct sockaddr {
    //        sa_family_t     sa_family;      /* Address family */
    //        char            sa_data[];      /* Socket address */
    //    };

    // sockaddr_in for IPv4 and sockaddr_in6 for IPv6 are extensions of sockaddr:

    //    Internet domain sockets:

    //    #include <netinet/in.h>

    //    struct sockaddr_in {
    //        sa_family_t     sin_family;     /* AF_INET */
    //        in_port_t       sin_port;       /* Port number */
    //        struct in_addr  sin_addr;       /* IPv4 address */
    //    };


    //    struct in_addr {
    //        in_addr_t s_addr;
    //    };



    // ------------- CONFIGURING THE ADDRESS OF THE SERVER ------------------------------

    // IP address of Google's server: 142.251.163.100

    char* ip = "127.0.0.1";   // IP address of the server in string format
    // This IP address needs to be converted to a network address structure in the af address family

    struct sockaddr_in address;

    address.sin_family = AF_INET;  // Address family for IPv4

    // Port on which the server is listening
    // address.sin_port = htons(80);  
    // Note: PORT number 80 is reserved for HTTP requests. This port number should be used 
    // when connecting to the google's server for example.

    // Attaching the port number on which the local server is running i.e. 2000
    address.sin_port = htons(2000);


    // address.sin_addr.s_addr;  // Server's IP address in uint_32 format

    // inet_pton - convert IPv4 and IPv6 addresses from text to binary form
    // pton stands for presentation to network

    // #include <arpa/inet.h>

    // int inet_pton(int af, const char *restrict src, void *restrict dst);

    // This function converts the character string src into a network
    // address structure in the af address family, then copies the
    // network address structure to dst.  The af argument must be either
    // AF_INET or AF_INET6.  dst is written in network byte order.

    inet_pton(AF_INET , ip , &address.sin_addr.s_addr);

    // --------- SERVER ADDRESS CONFIGURATION COMPLETED ----------------------------------


    // ------------------- CONNECTING TO THE SERVER --------------------------------------

    int connectionFeedback = connect(socketFd , (struct sockaddr*) &address , sizeof(address));

    if(connectionFeedback == -1)
    {
        printf("Could not connect to server !\n");
        return 2;
    }

    if(connectionFeedback == 0)
    {
        printf("Connection to server successful !\n\n");
    }

    // ------------------- CONNECTION TO SERVER ESTABLISHED -----------------------------


    // ------------------- COMMUNICATING WITH GOOGLE's SERVER ----------------------------


    // Sending an HTTP request to the Google's server

    // char* httpRequest = "GET \\ HTTP/1.1\r\nHOST:google.com\r\n\r\n";

    // send(socketFd , httpRequest , strlen(httpRequest) , 0);

    // Receiving response from the Google's server

    // char httpResponse[1024];

    // recv(socketFd , httpResponse , sizeof(httpResponse) , 0);

    // Displaying the Response received from Google's server

    // printf("Response received from Google's server: \n");

    // printf("%s\n" , httpResponse);

    // ------------------- COMMUNICATING WITH GOOGLE's SERVER --------------------------------



    // -------------- COMMUNICATING WITH THE LOCAL SERVER RUNNING ON PORT 2000 --------------

    // Receiving messages from the server
    startReceivingMessagesFromTheServer(socketFd);

    // Sending a message to the local server running on port 2000
    sendMessagesToTheServer(socketFd);

    close(socketFd);

    // Recieving a response from the Local Server running on port 2000

    // char responseFromLocalServer[1024];

    // recv(socketFd , responseFromLocalServer , sizeof(responseFromLocalServer) , 0);

    // Displaying the response recieved from the local server

    // printf("Response received from the Local Server: \n");

    // printf("%s\n" , responseFromLocalServer);

    // ---------- COMMUNICATING WITH THE LOCAL SERVER RUNNING ON PORT 2000 ------------------

    return 0;
}