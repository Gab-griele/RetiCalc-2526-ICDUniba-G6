#if defined _WIN32
    #include <winsock2.h>
#else
    #define closesocket close
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFERSIZE 512
#define PROTOPORT 27015

void ErrorHandler(char *errorMessage) {
    printf("%s", errorMessage);
}

void ClearWinSock() {
    #if defined _WIN32
        WSACleanup();
    #endif
}

int main(void) {
    #if defined _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            printf("error at WSASturtup\n");
            return -1;
        }
    #endif

    int Csocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (Csocket < 0) {
        ErrorHandler("socket creation failed.\n");
        ClearWinSock();
        return -1;
    }

    struct sockaddr_in sad;
    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = inet_addr("127.0.0.1");
    sad.sin_port = htons(PROTOPORT);

    // 1) Richiede connessione
    if (connect(Csocket, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
        ErrorHandler("Failed to connect.\n");
        closesocket(Csocket);
        ClearWinSock();
        return -1;
    }

    char buf[BUFFERSIZE];
    int bytesRcvd;

    // 1) Il client invia il messaggio iniziale "Hello" al server
    char *helloMsg = "Hello";
    send(Csocket, helloMsg, strlen(helloMsg), 0);

    // Lettura input utente
    char str1[BUFFERSIZE];

    printf("Inserisci la stringa da inviare: ");
    scanf("%s", str1);

    // 3) Il client invia la stringa al server
    send(Csocket, str1, strlen(str1), 0);

    printf("Dati inviati. Attendo risposta...\n");

    // 5) Il client legge la risposta inviata dal server
    bytesRcvd = recv(Csocket, buf, BUFFERSIZE - 1, 0);
    if (bytesRcvd > 0) {
        buf[bytesRcvd] = '\0';
        printf("Risposta dal server (senza vocali): %s\n", buf);
    }

    // Chiusura
    closesocket(Csocket);
    ClearWinSock();
    printf("\nConnessione chiusa.\n");
#if defined _WIN32
    system("pause");
#endif
    return 0;
}