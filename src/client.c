#include "pub_utils.h"
#include "socket_utils.h"

int sock;

// Handler di client.c per il segnale SIGINT
void sigint_handler(int sig) {
    // Invio un messaggio speciale al cameriere per indicare che il client sta terminando
    send(sock, "client_terminate", strlen("client_terminate"), 0);
    close(sock);
    exit(EXIT_SUCCESS);
}

int main() {
    signal(SIGINT, sigint_handler);

    // Creazione del socket del client e connessione al cameriere
    sock = connect_to_address(PUB_IP, CAMERIERE_PORT);
    set_socket_option(sock);

    printf("Sono entrato nel pub!\nChiedo se c'è un tavolo disponibile...\n");
    sleep(2);

    char message[MESSAGE_SIZE];

    // Chiedo al cameriere se c'è un tavolo disponibile
    strcpy(message, "C'è un tavolo disponibile?");
    send(sock, message, strlen(message), 0);

    memset(message, 0, MESSAGE_SIZE);

    // Leggo la risposta del cameriere
    receive_message(sock, message, MESSAGE_SIZE);
    handle_terminate_generic(sock, message);
    printf("%s\n", message);

    // Se non ci sono tavoli disponibili, chiudo la connessione e termino l'esecuzione
    if (strcmp(message, "Mi dispiace, non ci sono tavoli disponibili.") == 0) {
        printf("Riprova più tardi.\n");
        close(sock);
        exit(EXIT_SUCCESS);
    }

    // Se, invece, ci sono tavoli disponibili, procedo con l'ordinazione e la consegno al cameriere
    printf("Inserisci il numero della portata desiderata (da 1 a 12) e conferma con INVIO.\nPer terminare l'ordine, digita 'exit'.\n");

    // Inizializza una stringa per contenere l'ordine
    char order[MESSAGE_SIZE] = "";
    // Stringa per l'input dell'utente
    char input[MESSAGE_SIZE];

    // Ciclo per acquisire l'ordine dall'utente
    do {
        printf("Portata: ");
        fgets(input, sizeof(input), stdin); // Ottengo l'input del cliente
        input[strcspn(input, "\n")] = '\0'; // Rimuovo il carattere di newline dall'input
        
        // Controlla se il cliente vuole concludere l'ordine
        if (strcmp(input, "exit") == 0) {
            // Se il cliente tenta di uscire senza aver ordinato nulla, chiedi di ordinare almeno una portata
            if (strlen(order) == 0) {
                printf("Devi ordinare almeno una portata.\n");
                continue; // Torna all'inizio del ciclo per richiedere la portata
            }
            break;
        }
        
        if (!is_valid_number(input, 1, 12)) {
            printf("Inserisci un numero compreso tra 1 e 12.\n");
            continue;
        }
        
        strcat(order, input); // Aggiungo la portata inserita all'ordine
        strcat(order, " ");   // Aggiungo uno spazio per separare le portate
    } while (1);


    // Rimuovo lo spazio finale prima di inviare l'ordine
    order[strlen(order) - 1] = '\0';

    // Invio l'ordine al cameriere
    send(sock, order, strlen(order), 0);
    printf("Hai consegnato il tuo ordine al cameriere, ora attendi che il pub lo prepari.\n");

    memset(message, 0, MESSAGE_SIZE);

    // Ricevo le portate dal cameriere
    receive_message(sock, message, MESSAGE_SIZE);
    handle_terminate_generic(sock, message);
    printf("Il cameriere ti ha appena servito ciò che hai ordinato e stai mangiando...\n");
    sleep(3);

    printf("Hai finito e stai uscendo dal pub.\n");
    sleep(2);
    // Comunico al cameriere che ho finito di consumare e lascio il pub
    strcpy(message, "exit");
    send(sock, message, strlen(message), 0);

    close(sock);
    return 0;
}
