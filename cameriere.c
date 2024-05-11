#include "pub_utils.h"
#include "socket_utils.h"

int main() {
    // Creazione del socket del server
    int server_sock = create_socket();
    set_socket_option(server_sock);
    bind_socket(server_sock, PUB_IP, CAMERIERE_PORT);
    listen_socket(server_sock);

    printf("Il cameriere è operativo...\n\n");

    char message[MESSAGE_SIZE];

    while (1) {
        struct sockaddr_in client_addr;
        int client_sock = accept_connection(server_sock, &client_addr);

        // Fork per gestire in modo concorrente i client
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            close(client_sock);
            continue;
        }

        if (pid == 0) {
            // Processo figlio
            close(server_sock);

            memset(message, 0, MESSAGE_SIZE);

            receive_message(client_sock, message, MESSAGE_SIZE);
            printf("E' arrivato un client che chiede: %s\nChiedo al pub...\n", message);
            sleep(1);

            // Inoltro la richiesta al pub
            int pub_sock = connect_to_address(PUB_IP, PUB_PORT);
            send(pub_sock, message, strlen(message), 0);

            memset(message, 0, MESSAGE_SIZE);

            receive_message(pub_sock, message, MESSAGE_SIZE);
            printf("Risposta del pub: %s", message);

            int tavolo_assegnato;
            if (!strncmp(message, "Sì, c'è il tavolo ", strlen("Sì, c'è il tavolo ")) == 0) {
                // Avvisa il cliente che non ci sono posti disponibili nel pub
                send(client_sock, message, strlen(message), 0);
            } else {
                sscanf(message, "Sì, c'è il tavolo %d disponibile.", &tavolo_assegnato);
                printf("Faccio accomodare il cliente e gli servo il menù...\n");
                sleep(2);

                // Invia il menù al cliente specificando il tavolo
                memset(message, 0, MESSAGE_SIZE);
                snprintf(message, sizeof(message), "Il cameriere ti ha fatto accomodare al tavolo %d.\n"
                                                "Ecco il menu':\n"
                                                "1. Burger\n"
                                                "2. Nonna assunta\n"
                                                "3. Hadoken\n"
                                                "4. Django\n"
                                                "5. Los Pollos\n"
                                                "6. Smashed One\n"
                                                "7. Smashed One Doppio Bacon\n"
                                                "8. Smashed One Extr3me\n"
                                                "9. Diego Armando Masardona\n"
                                                "10. Spring Mania Veg\n"
                                                "11. Genovese Astrospaziale\n"
                                                "12. Nennella MOP\n", tavolo_assegnato);
                send(client_sock, message, strlen(message), 0);
                
                memset(message, 0, MESSAGE_SIZE);

                receive_message(client_sock, message, MESSAGE_SIZE);
                printf("Ordine ritirato dal tavolo %d: %s.\nLo invio al pub...\n", tavolo_assegnato, message);
                sleep(2);
                
                // Inoltra l'ordine al pub
                send(pub_sock, message, strlen(message), 0);
                
                memset(message, 0, MESSAGE_SIZE);

                receive_message(pub_sock, message, MESSAGE_SIZE);
                printf("Avviso dal pub: %s\nVado a servirlo...\n", message);
                sleep(2);
                
                // Invio la conferma al cliente
                send(client_sock, message, strlen(message), 0);
                
                memset(message, 0, MESSAGE_SIZE);
                
                receive_message(client_sock, message, MESSAGE_SIZE);
                printf("Il cliente al tavolo %d ha lasciato il pub, pulisco il tavolo...\n", tavolo_assegnato);
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
