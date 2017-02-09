#include <stdio.h>      
#include <sys/socket.h> 
#include <arpa/inet.h>  
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <stdbool.h>
#include "file.h"

#define ECHOMAX 255     /* Longest string to echo */
#define REMOVE_CLIENT 2

char **onlineIps;

void DieWithError(char *errorMessage) {
    perror(errorMessage);
    exit(1);
}

char **onlineIps = NULL;
uint16_t serverClients = 0;

bool alreadyActive(char *ip) {
    for(uint16_t i = 0; i < serverClients; i++)
        if(!strcmp(ip, onlineIps[i]))
            return true;
    return false;
}

void addNewIP(char *s) {
    //if(alreadyActive(s))
    //  return;
    if(alreadyActive(s))
        return;
    serverClients++;
    // printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));
    if(serverClients == 1) {
        onlineIps = (char **)malloc(sizeof(char*));
        onlineIps[0] = (char*)malloc(sizeof(char) * 11);
        strcpy(onlineIps[0], s);
    } else {
        onlineIps = (char **)realloc(onlineIps, serverClients * sizeof(char*));
        onlineIps[serverClients - 1] = (char*) malloc(sizeof(char) * 11);
        strcpy(onlineIps[serverClients - 1], s);
    }
    //system("clear");
}

bool removeIp(char *ip) {
    for(uint16_t i = 0; i < serverClients; i++) {
        if(!strcmp(ip, onlineIps[i])) {
            onlineIps[i] = NULL;
            free(onlineIps[i]);
            return true;
        }
    }
    return false;
}

bool isCommand(char *cmd) {
    char **strings = tokenizer(cmd, " ");
    return !strcmp(strings[0], "cmd");
}

uint16_t commandType(char *cmd) {
    if(!strcmp(cmd, "remove"))
        return REMOVE_CLIENT;
}

void initializeServer(void);

int main(int argc, char *argv[])
{
    int sock;                        /* Socket */
    struct sockaddr_in echoServAddr; /* Local address */
    struct sockaddr_in echoClntAddr; /* Client address */
    struct sockaddr_in echoClntAddr1;
    unsigned int cliAddrLen, cliAddrLen1;         /* Length of incoming message */
    char echoBuffer[ECHOMAX];        /* Buffer for echo string */
    unsigned short echoServPort;     /* Server port */
    int recvMsgSize;                 /* Size of received message */

    if (argc != 2)         /* Test for correct number of parameters */
    {
        fprintf(stderr,"Usage:  %s <UDP SERVER PORT>\n", argv[0]);
        exit(1);
    }

    echoServPort = atoi(argv[1]);  /* First arg:  local port */

    /* Create socket for sending/receiving datagrams */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echoServAddr.sin_port = htons(echoServPort);      /* Local port */

    
    if (bind(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("bind() failed");

    for (;;)
    {
        cliAddrLen = sizeof(echoClntAddr);
        memset(echoBuffer, 0, sizeof(echoBuffer));
        /* Block until receive message from a client */
        if ((recvMsgSize = recvfrom(sock, echoBuffer, ECHOMAX, 0,
            (struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
                DieWithError("recvfrom() failed");
        else {
            printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));
            char **tokens = tokenizer(echoBuffer, " ");
            if(isCommand(tokens[0])) {
                uint16_t type = commandType(tokens[1]);
                if(type == REMOVE_CLIENT)
                    removeIp(tokens[2]);
                printf("%d\n", type);
            }
            echoClntAddr.sin_port = htons(5001);
            char message[ECHOMAX + 1];
            strcpy(message, inet_ntoa(echoClntAddr.sin_addr));
            strcat(message, " diz: ");
            strcat(message,echoBuffer);
            addNewIP(inet_ntoa(echoClntAddr.sin_addr));
            for(int i = 0; i < serverClients; i++) {
                if(onlineIps[i] != NULL) {
                    echoClntAddr.sin_addr.s_addr = inet_addr(onlineIps[i]);
                    if (sendto(sock, message, sizeof(message), 0,
                         (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != sizeof(message))
                            DieWithError("sendto() sent a different number of bytes than expected");
                }
            }
        }

    }
}
