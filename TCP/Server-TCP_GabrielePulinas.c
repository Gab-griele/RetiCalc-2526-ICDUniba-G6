#if defined _WIN32    // Usa _WIN32 invece di WIN32
#include <winsock2.h> // Usa winsock2.h per compatibilità moderna
typedef int socklen_t;
#else
#define closesocket close
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>     // for atoi()
#include <string.h>     // Necessario per memset()
#include <ctype.h> // Necessario per toupper() e tolower()

#define PROTOPORT 27015 // default protocol port number
#define QLEN 6          // size of request queue
#define BUFFERSIZE 512

void ErrorHandler(char *errorMessage)
{
    printf("%s", errorMessage);
}
void ClearWinSock()
{
#if defined _WIN32
    WSACleanup();
#endif
}

int main(int argc, char *argv[])
{
    int port;
    if (argc > 1)
    {
        // Corretto il commento qui sotto
        port = atoi(argv[1]); // if argument specified convert argument to binary
    }
    else
        port = PROTOPORT; // use default port number
    if (port < 0)
    {
        printf("bad port number %s \n", argv[1]);
        return 0;
    }
#if defined _WIN32 // initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        ErrorHandler("Error at WSAStartup()\n");
        return 0;
    }
#endif
    // CREAZIONE DELLA SOCKET
    int MySocket;
    MySocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (MySocket < 0)
    {
        ErrorHandler("socket creation failed.\n");
        ClearWinSock();
        return -1;
    }
    // ASSEGNAZIONE DI UN INDIRIZZO ALLA SOCKET
    struct sockaddr_in sad;
    memset(&sad, 0, sizeof(sad)); // ensures that extra bytes contain 0
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = inet_addr("127.0.0.1");
    sad.sin_port = htons(port);

    if (bind(MySocket, (struct sockaddr *)&sad, sizeof(sad)) < 0)
    {
        ErrorHandler("bind() failed.\n");
        closesocket(MySocket);
        ClearWinSock();
        return -1;
    }
    // SETTAGGIO DELLA SOCKET ALL'ASCOLTO
    if (listen(MySocket, QLEN) < 0)
    {
        ErrorHandler("listen() failed.\n");
        closesocket(MySocket);
        ClearWinSock();
        return -1;
    }
    // ACCETTARE UNA NUOVA CONNESSIONE
    struct sockaddr_in cad; // structure for the client address
    int clientSocket;       // socket descriptor for the client
    socklen_t clientLen;    // the size of the client address
    char buf1[BUFFERSIZE];
    char buf2[BUFFERSIZE];

    printf("Server in ascolto...\n");

    while (1)
    {
        clientLen = sizeof(cad); // set the size of the client address
        clientSocket = accept(MySocket, (struct sockaddr *)&cad, &clientLen);

        if (clientSocket < 0)
        {
            ErrorHandler("accept() failed.\n");
            continue; // Torna ad ascoltare
        }

        // 2) Ricevuti i dati client (Hello), il server visualizza l'IP
        int bytesRcvd = recv(clientSocket, buf1, BUFFERSIZE - 1, 0);
        if (bytesRcvd > 0) {
            buf1[bytesRcvd] = '\0';
            // Controllo se è "Hello" (opzionale ma da specifica il client invia Hello)
            // Visualizza messaggio con IP
            printf("ricevuti dati dal client con indirizzo: %s\n", inet_ntoa(cad.sin_addr));
        }

        // 4) Il server legge la stringa inviata dal client
        bytesRcvd = recv(clientSocket, buf1, BUFFERSIZE - 1, 0);
        if (bytesRcvd > 0)
        {
            buf1[bytesRcvd] = '\0';
            printf("Stringa ricevuta: %s\n", buf1);

            // Elimina tutte le vocali
            int j = 0;
            for (int i = 0; buf1[i] != '\0'; i++) {
                char c = tolower(buf1[i]);
                if (c != 'a' && c != 'e' && c != 'i' && c != 'o' && c != 'u') {
                    buf1[j++] = buf1[i];
                }
            }
            buf1[j] = '\0';

            // Invia nuovamente al client
            send(clientSocket, buf1, strlen(buf1), 0);
        }

        printf("Risposta inviata. Chiudo connessione con questo client.\n");
        closesocket(clientSocket);
    }
}