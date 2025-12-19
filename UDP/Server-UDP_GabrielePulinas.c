#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if defined WIN32 || defined _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#define closesocket close
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h> // Per gethostbyaddr
#endif

#define ECHOMAX 255 // [cite: 127]
#define PORT 48000  // Porta di default

void ErrorHandler(char *errorMessage) {
    perror(errorMessage);
    exit(1);
}

void ClearWinSock() {
#if defined WIN32 || defined _WIN32
    WSACleanup();
#endif
}

// Funzione per rimuovere le vocali (Step 5 del protocollo)
void rimuoviVocali(char *str) {
    int i = 0, j = 0;
    while (str[i]) {
        char c = tolower(str[i]);
        if (c != 'a' && c != 'e' && c != 'i' && c != 'o' && c != 'u') {
            str[j++] = str[i];
        }
        i++;
    }
    str[j] = '\0';
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
    struct sockaddr_in echoClntAddr;
    socklen_t cliAddrLen;
    char echoBuffer[ECHOMAX];
    int recvMsgSize;
    struct hostent *hostInfo; // [cite: 263]

    // Creazione della socket UDP [cite: 149]
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        ErrorHandler("socket() failed");

    // Costruzione indirizzo server
    memset(&echoServAddr, 0, sizeof(echoServAddr));
    echoServAddr.sin_family = AF_INET;
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    echoServAddr.sin_port = htons(PORT);

    // Bind [cite: 158]
    if (bind(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
        ErrorHandler("bind() failed");

    printf("Server in ascolto sulla porta %d...\n", PORT);

    while (1) {
        cliAddrLen = sizeof(echoClntAddr);

        // Ricezione dati
        recvMsgSize = recvfrom(sock, echoBuffer, ECHOMAX, 0, (struct sockaddr *)&echoClntAddr, &cliAddrLen);
        if (recvMsgSize < 0) ErrorHandler("recvfrom() failed");

        echoBuffer[recvMsgSize] = '\0'; // Termina la stringa

        // Risoluzione info client (Step 3)
        hostInfo = gethostbyaddr((char *)&echoClntAddr.sin_addr, 4, AF_INET);
        char *clientName = (hostInfo != NULL) ? hostInfo->h_name : "Sconosciuto";
        char *clientIP = inet_ntoa(echoClntAddr.sin_addr);

        // Se il messaggio è "Hello", stampiamo solo le info (Step 3) MA NON RISPONDIAMO
        if (strcmp(echoBuffer, "Hello") == 0) {
            printf("ricevuti dati dal client nome: %s indirizzo: %s\n", clientName, clientIP);
            printf("Messaggio di saluto ricevuto: %s\n", echoBuffer);
            // NON c'è sendto() qui, il server aspetta il prossimo messaggio
        } 
        else {
            // Se NON è "Hello", è la stringa da elaborare (Step 5)
            printf("Stringa da elaborare ricevuta: %s\n", echoBuffer);

            // Togli le vocali
            rimuoviVocali(echoBuffer);

            // Invia la risposta SOLO ORA
            if (sendto(sock, echoBuffer, strlen(echoBuffer), 0, (struct sockaddr *)&echoClntAddr, sizeof(echoClntAddr)) != strlen(echoBuffer))
                ErrorHandler("sendto() sent different number of bytes than expected");
                
            printf("Risposta inviata: %s\n", echoBuffer);
        }
    }
    
    // Non raggiungibile nel while(1), ma buona prassi
    closesocket(sock);
    ClearWinSock();
    return 0;
}