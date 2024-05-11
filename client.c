#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include "socket_utils.h"

#define PUB_IP "127.0.0.1"
#define CAMERIERE_PORT 8888

// Funzione per verificare se la stringa è un numero compreso tra min e max
int is_valid_number(const char *str, int min, int max) {
    int num = atoi(str);
    return num >= min && num <= max;
}

int main() {
    int sock = create_socket(); // Creazione del socket del client
    set_socket_option(sock);    // Impostazione delle opzioni del socket

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(CAMERIERE_PORT);
    inet_pton(AF_INET, PUB_IP, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Errore durante la connessione al cameriere");
        exit(EXIT_FAILURE);
    }

    printf("Benvenuto al pub!\n\n");

    // Chiedo al cameriere se c'è un tavolo disponibile
    char *message = malloc(1024 * sizeof(char));
    strcpy(message, "C'è un tavolo disponibile?");
    send(sock, message, strlen(message), 0);

    // Leggo la risposta del cameriere
    char response[1024];
    ssize_t bytes_received = recv(sock, response, sizeof(response), 0);
    if (bytes_received == -1) {
        perror("recv");
        free(message);
        close(sock);
        exit(EXIT_FAILURE);
    }
    response[bytes_received] = '\0';
    printf("%s\n", response);

    // Se non ci sono tavoli disponibili...
    if (strcmp(response, "Mi dispiace, non ci sono tavoli disponibili.") == 0) {
        printf("Riprova più tardi.\n");
        free(message);
        close(sock);
        exit(EXIT_SUCCESS);
    }

    // Se, invece, ci sono tavoli disponibili, procedi con l'ordinazione e consegnala al cameriere
    printf("Inserisci il numero della portata desiderata (da 1 a 12) e conferma con INVIO. Per terminare, digita 'exit'.\n");

    char order[1024] = "";
    char input[1024];
    do {
        printf("Portata: ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0) {
            break; // Uscita dal ciclo se l'utente inserisce "exit"
        }

        if (!is_valid_number(input, 1, 12)) {
            printf("Inserisci un numero compreso tra 1 e 12.\n");
            continue; // Salta questa iterazione se l'input non è valido
        }

        strcat(order, input); // Aggiungi la portata all'ordine
        strcat(order, " ");   // Aggiungi uno spazio per separare le portate
    } while (1);

    // Rimuove lo spazio finale prima di inviare l'ordine
    order[strlen(order) - 1] = '\0';

    // Invia l'ordine al cameriere
    send(sock, order, strlen(order), 0);
    printf("Hai consegnato il tuo ordine al cameriere, ora attendi che il pub lo prepari.\n");

    // Riceve l'ordine dal cameriere
    char ordine_servito[1024];
    ssize_t bytes_received2 = recv(sock, ordine_servito, sizeof(ordine_servito), 0);
    if (bytes_received2 == -1) {
        perror("recv");
        free(message);
        close(sock);
        exit(EXIT_FAILURE);
    }
    ordine_servito[bytes_received2] = '\0';
    printf("Il cameriere ti ha appena servito ciò che hai ordinato, buon appetito!\n");

    // Comunica al cameriere che hai finito di consumare e lascia il pub
    strcpy(message, "exit");
    send(sock, message, strlen(message), 0);

    free(message);
    close(sock);
    return 0;
}
