#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined WIN32 || defined _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#define closesocket close
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h> 
#endif

#define ECHOMAX 255

void ErrorHandler(char *errorMessage) {
    perror(errorMessage);
    exit(1);
}

void ClearWinSock() {
#if defined WIN32 || defined _WIN32
    WSACleanup();
#endif
}

int main() {
#if defined WIN32 || defined _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Error at WSAStartup\n");
        return EXIT_FAILURE;
    }
#endif

    int sock;
    struct sockaddr_in echoServAddr;
    struct sockaddr_in fromAddr;
    socklen_t fromSize;
    char echoString[ECHOMAX];
    char echoBuffer[ECHOMAX];
    int respStringLen;
    char serverHostname[100];
    int serverPort;
    struct hostent *serverEnt; // [cite: 263]
    struct hostent *fromEnt;

    // 1. Lettura Hostname e Porta
    printf("Inserisci il nome dell'host server (es. localhost): ");
    scanf("%s", serverHostname);
    printf("Inserisci il numero di porta (es. 48000): ");
    scanf("%d", &serverPort);

    // Risoluzione del nome del server (DNS Lookup) [cite: 253, 257]
    serverEnt = gethostbyname(serverHostname);
    if (serverEnt == NULL) {
        fprintf(stderr, "gethostbyname() fallito per %s\n", serverHostname);
        exit(1);
    }

    // Creazione Socket [cite: 216]
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        ErrorHandler("socket() failed");

    // Costruzione indirizzo server usando l'IP risolto dal DNS
    memset(&echoServAddr, 0, sizeof(echoServAddr));
    echoServAddr.sin_family = AF_INET;
    // Copiamo l'indirizzo ottenuto da gethostbyname nella struttura address [cite: 268, 331]
    memcpy(&echoServAddr.sin_addr, serverEnt->h_addr_list[0], serverEnt->h_length);
    echoServAddr.sin_port = htons(serverPort);

    // 2. Invio messaggio "Hello" (Step 2)
    char *helloMsg = "Hello";
    sendto(sock, helloMsg, strlen(helloMsg), 0, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)); // [cite: 226]
    
    // Il protocollo non specifica che dobbiamo aspettare una risposta all'Hello,
    // ma che il server lo stampa. Procediamo all'invio della stringa.

    // 4. Lettura stringa da standard input
    printf("Inserisci la stringa da inviare: ");
    scanf("%s", echoString); // Nota: scanf legge fino allo spazio. Usa fgets per frasi intere.

    if (strlen(echoString) > ECHOMAX)
        ErrorHandler("Stringa troppo lunga");

    // Invio stringa al server [cite: 86]
    if (sendto(sock, echoString, strlen(echoString), 0, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) != strlen(echoString))
        ErrorHandler("sendto() sent different number of bytes than expected");

    // 7. Ricezione risposta (Step 7)
    fromSize = sizeof(fromAddr);
    respStringLen = recvfrom(sock, echoBuffer, ECHOMAX, 0, (struct sockaddr *)&fromAddr, &fromSize); // [cite: 103, 231]

    if (respStringLen < 0) ErrorHandler("recvfrom() failed");
    echoBuffer[respStringLen] = '\0';

    // Risoluzione inversa: Dall'IP di chi ha risposto al Nome Simbolico [cite: 273, 277]
    fromEnt = gethostbyaddr((char *)&fromAddr.sin_addr, 4, AF_INET);
    char *fromName = (fromEnt != NULL) ? fromEnt->h_name : "Sconosciuto";

    // Visualizzazione output come richiesto
    printf("Stringa %s ricevuta dal server nome:%s indirizzo:%s\n", 
           echoBuffer, 
           fromName, 
           inet_ntoa(fromAddr.sin_addr));

    closesocket(sock);
    ClearWinSock();
    return 0;
}