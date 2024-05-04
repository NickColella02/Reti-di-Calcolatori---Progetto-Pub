#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#define CAMERIERE_PORT 8081
#define PUB_PORT 8080
#define MAX_CLIENTS 5
#define SERVER_IP "127.0.0.1"

int main() {
    struct sockaddr_in serv_addr, pub_addr, client_addr;
    char buffer[1024] = {0};
    int server_fd, pub_fd, client_fd;
    int opt = 1;
    int addrlen = sizeof(serv_addr);
    int pub_addrlen = sizeof(pub_addr);
    int client_addrlen = sizeof(client_addr);
    int attempts = 0;

    printf("Il cameriere è operativo...\n");

    // Creazione del socket del cameriere
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Errore nella creazione del socket");
        exit(EXIT_FAILURE);
    }

    // Settaggio dell'opzione del socket
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Errore nel settaggio dell'opzione del socket");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(CAMERIERE_PORT);

    // Bind del socket del cameriere
    if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0) {
        perror("Errore durante il bind");
        exit(EXIT_FAILURE);
    }

    // Ascolto delle connessioni
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Errore durante l'ascolto");
        exit(EXIT_FAILURE);
    }

    // Accettazione delle connessioni e gestione delle richieste dei clienti
    while (1) {
        if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t*)&client_addrlen))<0) {
            perror("Errore durante l'accettazione della connessione");
            exit(EXIT_FAILURE);
        }

        // Connessione al server del pub
        if ((pub_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Errore nella creazione del socket");
            exit(EXIT_FAILURE);
        }

        pub_addr.sin_family = AF_INET;
        pub_addr.sin_port = htons(PUB_PORT);

        // Converti l'indirizzo IP da stringa a binario e assegnalo a sin_addr
        if(inet_pton(AF_INET, SERVER_IP, &pub_addr.sin_addr)<=0) {
            perror("Indirizzo non valido/non supportato");
            exit(EXIT_FAILURE);
        }

        // Tentativi di connessione al server del pub
        while (connect(pub_fd, (struct sockaddr *)&pub_addr, sizeof(pub_addr)) < 0) {
            if (errno == ECONNREFUSED && attempts < 10) {
                attempts++;
                sleep(1); // Attendi un secondo prima di ritentare la connessione
            } else {
                perror("Errore durante la connessione al pub");
                exit(EXIT_FAILURE);
            }
        }

        // Richiesta di posti disponibili al pub
        printf("E' arrivato un cliente, chiedo se nel pub ci sono posti disponibili...\n");
        char read_buffer[1024] = {0};
        char write_buffer[1024] = {0};
        memset(read_buffer, 0, sizeof(read_buffer));
        memset(write_buffer, 0, sizeof(write_buffer));
        read(pub_fd, read_buffer, 1024);
        strcpy(buffer, read_buffer);

        // Se il pub ha confermato la disponibilità, fai accedere il cliente al pub
        if (strcmp(buffer, "Posto disponibile") == 0) {
            printf("Il pub ha confermato la disponibilita', faccio accomodare il cliente e gli servo il menu'...\n");
        } else {
            printf("Il pub non ha confermato la disponibilita', avviso il cliente...\n");
        }

        // Inoltra la risposta del pub al cliente
        memset(write_buffer, 0, sizeof(write_buffer));
        strcpy(write_buffer, read_buffer);
        send(client_fd, write_buffer, strlen(write_buffer), 0);

        // Ricezione dell'ordine dal cliente
        memset(read_buffer, 0, sizeof(read_buffer));
        memset(write_buffer, 0, sizeof(write_buffer));
        read(client_fd, read_buffer, 1024);
        strcpy(buffer, read_buffer);
        printf("Ordine ricevuto dal cliente: %s\n", buffer);

        // Inoltra l'ordine al pub
        memset(write_buffer, 0, sizeof(write_buffer));
        strcpy(write_buffer, read_buffer);
        send(pub_fd, write_buffer, strlen(write_buffer), 0);

        // Ricezione della conferma di ricezione dell'ordine dal pub
        memset(read_buffer, 0, sizeof(read_buffer));
        memset(write_buffer, 0, sizeof(write_buffer));
        read(pub_fd, read_buffer, 1024);
        strcpy(buffer, read_buffer);
        printf("Conferma di ricezione dell'ordine dal pub: %s\n", buffer);

        close(pub_fd);
        close(client_fd);
    }
    return 0;
}
