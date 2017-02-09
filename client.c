/*
* User can send commands to the app: cmd <command name>
* cmd clear: clear the console
* cmd exit: it removes the client from the chat
*
* Some of these commands need to be executed on client side(i.e clear) and others need to be executed on server side(i.e exit, the servers
* must know who is out and remove its ip from his list of active ips);
*/

#define CLEAR_SCREEN 0
#define EXIT_SERVICE 1


#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() and alarm() */
#include <errno.h>      /* for errno and EINTR */
#include <signal.h>     /* for sigaction() */
#include <pthread.h>
#include "file.h"
#include <stdbool.h>

const char *servIP;                    /* IP address of server */

/*send a more complex struct, not only a simple string*/

//typedef struct Message{
//    const char *message;
//    const char *ipSender;
//    size_t lenMessage;  
//};

#define ECHOMAX 255

void DieWithError(char *errorMessage) {
    perror(errorMessage);
    exit(1);
}

bool isCommand(char *cmd) {
    char **strings = tokenizer(cmd, " ");
    return !strcmp(strings[0], "cmd");
}

uint16_t commandType(char *cmd) {
    if(!strcmp(cmd, "clear"))
        return CLEAR_SCREEN;
    if(!strcmp(cmd, "exit"))
        return EXIT_SERVICE;
}

void executeCommand(uint16_t type){
    switch(type) {
        case CLEAR_SCREEN:
            system("clear");
            break;
        case EXIT_SERVICE:
            exit(0);
            break;
        default:
            puts("illegal command");
            break;
    }
}

int sock;
unsigned short port;
void *sendMessageServer(void *message) {
                          /* Socket descriptor */
    struct sockaddr_in echoServAddr; /* Echo server address */
    struct sockaddr_in fromAddr;     /* Source address of echo */
    unsigned short echoServPort;     /* Echo server port */
    unsigned int fromSize;           /* In-out of address size for recvfrom() */
    //char *echoString = "risos";                /* String to send to echo server */
    char echoBuffer[ECHOMAX+1];      /* Buffer for receiving echoed string */
    int echoStringLen;               /* Length of string to echo */
    int respStringLen;               /* Length of received response */

              /* First arg: server IP address (dotted quad) */

    echoServPort = 5000;
    /* Create a datagram/UDP socket */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));    /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                 /* Internet addr family */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);  /* Server IP address */
    echoServAddr.sin_port   = htons(echoServPort);     /* Server port */
    for(;;){
        
        char *echoString = getString();
        if(isCommand(echoString)) {
            char *cmd = (tokenizer(echoString, " "))[1];
            uint16_t cmdType = commandType(cmd);
            executeCommand(cmdType); 
        }
        echoStringLen = strlen(echoString);
        if ((echoStringLen = strlen(echoString)) > ECHOMAX)  /* Check input length */
            DieWithError("Echo word too long");

    
        if (sendto(sock, echoString, echoStringLen, 0, (struct sockaddr *)
                   &echoServAddr, sizeof(echoServAddr)) != echoStringLen)
            DieWithError("sendto() sent a different number of bytes than expected");
      
    }

    close(sock);
    exit(0);
}

void* receiveMessageFromServer(void *arg) {
    
    struct sockaddr_in addr;
    int sock1;
    /* Create socket for sending/receiving datagrams */
    if ((sock1 = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    memset(&addr, 0, sizeof(addr));   
    addr.sin_family = AF_INET;                
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(5001);
    char echoBuffer[ECHOMAX + 1];
    if (bind(sock1, (struct sockaddr *) &addr, sizeof(addr)) < 0)
        DieWithError("bind() failed");
    for(;;) {
                /* Set the size of the in-out parameter */
        socklen_t cliAddrLen = sizeof(addr);
        size_t recvMsgSize;
        /* Block until receive message from a client */
        memset(echoBuffer, 0, sizeof(echoBuffer)); /*clear old messages*/
        if ((recvMsgSize = recvfrom(sock1, echoBuffer, ECHOMAX, 0,
            (struct sockaddr *) &addr, &cliAddrLen)) < 0)
                DieWithError("recvfrom() failed");
        else 
            printf("%s\n", echoBuffer);
    }
}

int main(int argc, char const *argv[])
{
    servIP = argv[1];
    port   = atoi(argv[2]);
    pthread_t t1, t2;
    
    pthread_create(&t1, NULL, sendMessageServer, NULL);
    pthread_create(&t2, NULL, receiveMessageFromServer, NULL);
    pthread_join(t1,NULL);
    pthread_join(t2,NULL); 
    return 0;
}
