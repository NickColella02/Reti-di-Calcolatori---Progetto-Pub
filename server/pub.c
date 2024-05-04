#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PUB_PORT 8080
#define MAX_CLIENTS 5

int main() {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *pub_free = "Posto disponibile";
    char *pub_full = "Posto non disponibile";

    printf("Il pub è operativo...\n");

    // Creazione del socket del server
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Errore nella creazione del socket");
        exit(EXIT_FAILURE);
    }

    // Settaggio dell'opzione del socket
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Errore nel settaggio dell'opzione del socket");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PUB_PORT);

    // Bind del socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("Errore durante il bind");
        exit(EXIT_FAILURE);
    }

    // Ascolto delle connessioni
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Errore durante l'ascolto");
        exit(EXIT_FAILURE);
    }

    // Accettazione delle connessioni e gestione degli ordini
    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("Errore durante l'accettazione della connessione");
            exit(EXIT_FAILURE);
        }

        // Verifica se ci sono posti disponibili
        int seats_available = 1; // Supponiamo che ci siano posti disponibili

        if (seats_available) {
            send(new_socket, pub_free, strlen(pub_free), 0); // Risposta positiva al cameriere
            printf("C'è un posto disponibile, puoi far accomodare il cliente.\n");
        } else {
            send(new_socket, pub_full, strlen(pub_full), 0); // Risposta negativa al cameriere
            printf("Non c'è posto disponibile per il cliente.\n");
        }

        // Ricezione dell'ordine dal cameriere
        char read_buffer[1024] = {0};
        memset(read_buffer, 0, sizeof(read_buffer));
        memset(buffer, 0, sizeof(buffer));
        read(new_socket, read_buffer, 1024);
        strcpy(buffer, read_buffer);
        printf("Ordine ricevuto dal cameriere: %s\n", buffer);

        // Preparazione dell'ordine
        printf("Preparazione dell'ordine in corso...\n");
        sleep(2); // Simulazione della preparazione

        // Invio della conferma di ricezione dell'ordine al cameriere
        char *order_ready = "Ordine pronto!";
        char write_buffer[1024] = {0};
        memset(write_buffer, 0, sizeof(write_buffer));
        strcpy(write_buffer, order_ready);
        send(new_socket, write_buffer, strlen(write_buffer), 0);

        close(new_socket);
    }
    return 0;
}
