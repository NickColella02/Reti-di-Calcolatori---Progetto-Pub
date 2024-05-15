#include "pub_utils.h"
#include "socket_utils.h"

int server_sock;
int client_sock;
int pub_sock;

// Handler di cameriere.c per il segnale SIGINT
void sigint_handler(int sig) {
    send(client_sock, "cameriere_terminate", strlen("cameriere_terminate"), 0);
    send(pub_sock, "cameriere_terminate", strlen("cameriere_terminate"), 0);
    close(server_sock);
    close(client_sock);
    close(pub_sock);
    exit(EXIT_SUCCESS);
}

int main() {
    signal(SIGINT, sigint_handler);

    // Creazione del socket
    server_sock = create_socket();
    set_socket_option(server_sock);
    bind_socket(server_sock, PUB_IP, CAMERIERE_PORT);
    listen_socket(server_sock);

    printf("Il cameriere è operativo...\n\n");

    char message[MESSAGE_SIZE];

    while (1) {
        struct sockaddr_in client_addr;
        client_sock = accept_connection(server_sock, &client_addr);

        // Fork per gestire le comunicazioni con i client
        pid_t pid = fork();
        if (pid == -1) {
            perror("Errore durante fork");
            close(client_sock);
            continue;
        }

        if (pid == 0) {
            // Processo figlio
            close(server_sock);

            pub_sock = connect_to_address(PUB_IP, PUB_PORT);

            memset(message, 0, MESSAGE_SIZE);

            // Ricevo la richiesta di posti disponibili dal cliente
            receive_message(client_sock, message, MESSAGE_SIZE);
            handle_terminate_for_cameriere(pub_sock, client_sock, message);
            printf("E' arrivato un client che chiede: %s\nChiedo al pub...\n", message);
            sleep(1);

            // Inoltro la richiesta al pub
            send(pub_sock, message, strlen(message), 0);

            memset(message, 0, MESSAGE_SIZE);

            // Ricevo la risposta del pub
            receive_message(pub_sock, message, MESSAGE_SIZE);
            handle_terminate_for_cameriere(pub_sock, client_sock, message);
            printf("Risposta del pub: %s", message);

            int tavolo_assegnato;
            if (!strncmp(message, "S", strlen("S")) == 0) {
                // Avvisa il cliente che non ci sono posti disponibili nel pub
                send(client_sock, message, strlen(message), 0);
            } else {
                // Memorizza dalla stringa il numero di tavolo assegnato
                sscanf(message, "Sì, c'è il tavolo %d disponibile.", &tavolo_assegnato);
                printf("Faccio accomodare il cliente e gli servo il menù...\n");
                sleep(2);

                memset(message, 0, MESSAGE_SIZE);

                // Invia il menù al cliente specificando il tavolo
                invia_menu(client_sock, message, tavolo_assegnato);
                
                memset(message, 0, MESSAGE_SIZE);

                // Ricevi l'ordine effettuato dal cliente
                receive_message(client_sock, message, MESSAGE_SIZE);
                handle_terminate_for_cameriere(pub_sock, client_sock, message);
                printf("Ordine ritirato dal tavolo %d: %s.\nLo invio al pub...\n", tavolo_assegnato, message);
                sleep(2);
                
                // Inoltra l'ordine al pub
                send(pub_sock, message, strlen(message), 0);
                
                memset(message, 0, MESSAGE_SIZE);

                // Ricevi l'avviso riguardo l'ordine pronto dal pub
                receive_message(pub_sock, message, MESSAGE_SIZE);
                handle_terminate_for_cameriere(pub_sock, client_sock, message);
                printf("Avviso dal pub: %s\nVado a servirlo...\n", message);
                sleep(2);
                
                // Servo l'ordine al cliente
                send(client_sock, message, strlen(message), 0);
                
                memset(message, 0, MESSAGE_SIZE);
                
                // Ricevi il messaggio dal client riguardo la sua uscita dal pub
                receive_message(client_sock, message, MESSAGE_SIZE);
                handle_terminate_for_cameriere(pub_sock, client_sock, message);
                printf("Il cliente al tavolo %d ha lasciato il pub, pulisco il tavolo e comunico al pub...\n", tavolo_assegnato);
                sleep(2);
                
                // Comunicalo al pub per fargli liberare il tavolo
                send(pub_sock, message, strlen(message), 0);
            } 
            close(client_sock);
            close(pub_sock);
            exit(EXIT_SUCCESS);
        } else {
            // Processo padre
            close(client_sock);
        }
    }

    close(server_sock);
    return 0;
}