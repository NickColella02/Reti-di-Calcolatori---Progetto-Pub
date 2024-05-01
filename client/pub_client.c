#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define SERVER_ADDR "127.0.0.1"

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    // Creazione del socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Errore nella creazione del socket \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Conversione dell'indirizzo IP da stringa a formato binario
    if(inet_pton(AF_INET, SERVER_ADDR, &serv_addr.sin_addr)<=0) {
        printf("\n Indirizzo non valido/indirizzo non supportato \n");
        return -1;
    }

    // Connessione al server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\n Connessione fallita \n");
        return -1;
    }

    // Lettura del messaggio di benvenuto dal server
    read(sock, buffer, 1024);
    printf("%s\n", buffer);

    // Lettura del menu' dal server
    read(sock, buffer, 1024);
    printf("%s\n", buffer);

    // Ciclo per selezionare elementi dal menu'
    while (1) {
        int choice;
        printf("Scegli qualcosa dal menu' (1-3) o premi 0 per terminare l'ordine: ");
        scanf("%d", &choice);

        if (choice == 0) {
            // Invio dell'ordine al server
            send(sock, "0\n", strlen("0\n"), 0);
            break;
        }

        // Invia l'elemento selezionato al server
        char item[100];
        sprintf(item, "%d\n", choice);
        send(sock, item, strlen(item), 0);
    }

    // Stampa un messaggio che indica che l'ordine è stato consegnato al cameriere
    printf("Hai consegnato il tuo ordine al cameriere, attendi che arrivi al tavolo.\n");

    return 0;
}