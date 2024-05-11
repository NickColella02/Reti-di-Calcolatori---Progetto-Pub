#include "pub_utils.h"
#include "socket_utils.h"

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
    sleep(2);

    char message[MESSAGE_SIZE]; // Variabile per inviare e ricevere messaggi

    // Chiedo al cameriere se c'è un tavolo disponibile
    strcpy(message, "C'è un tavolo disponibile?");
    send(sock, message, strlen(message), 0);

    // Pulizia della variabile message prima di leggere la risposta del cameriere
    memset(message, 0, MESSAGE_SIZE);

    // Leggo la risposta del cameriere
    receive_message(sock, message, MESSAGE_SIZE);
    printf("%s\n", message);

    // Se non ci sono tavoli disponibili...
    if (strcmp(message, "Mi dispiace, non ci sono tavoli disponibili.") == 0) {
        printf("Riprova più tardi.\n");
        close(sock);
        exit(EXIT_SUCCESS);
    }

    // Se, invece, ci sono tavoli disponibili, procedi con l'ordinazione e consegnala al cameriere
    printf("Inserisci il numero della portata desiderata (da 1 a 12) e conferma con INVIO. Per terminare, digita 'exit'.\n");

    char order[MESSAGE_SIZE] = "";
    char input[MESSAGE_SIZE];
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

    // Pulizia della variabile message prima di leggere l'ordine servito dal cameriere
    memset(message, 0, MESSAGE_SIZE);

    // Riceve l'ordine dal cameriere
    receive_message(sock, message, MESSAGE_SIZE);
    printf("Il cameriere ti ha appena servito ciò che hai ordinato e stai mangiando...\n");
    sleep(3);

    printf("Hai finito e stai uscendo dal pub.\n");
    sleep(2);
    // Comunica al cameriere che hai finito di consumare e lascia il pub
    strcpy(message, "exit");
    send(sock, message, strlen(message), 0);

    close(sock);
    return 0;
}
