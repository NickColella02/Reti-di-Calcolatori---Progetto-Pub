#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#define PUB_IP "127.0.0.1"
#define PUB_PORT 8889
#define CAMERIERE_PORT 8888

int main() {
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("Errore nella creazione del socket");
        exit(EXIT_FAILURE);
    }

    int enable = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("Errore nel settaggio dell'opzione del socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr, client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(CAMERIERE_PORT);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Errore durante il bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, 5) == -1) {
        perror("Errore durante l'ascolto");
        exit(EXIT_FAILURE);
    }

    printf("Il cameriere è operativo...\n\n");

    while (1) {
        int client_sock;
        int client_len = sizeof(client_addr);
        if ((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, (socklen_t *)&client_len)) == -1) {
            perror("Errore durante l'accettazione");
            continue;
        }

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

            char *request = malloc(1024 * sizeof(char));
            ssize_t bytes_received = recv(client_sock, request, 1024, 0);
            if (bytes_received == -1) {
                perror("recv");
                free(request);
                close(client_sock);
                exit(EXIT_FAILURE);
            }
            request[bytes_received] = '\0';
            printf("E' arrivato un client che chiede: %s\n", request);

            // Inoltro la richiesta al pub
            int pub_sock = socket(AF_INET, SOCK_STREAM, 0);
            if (pub_sock == -1) {
                perror("socket");
                exit(EXIT_FAILURE);
            }

            struct sockaddr_in pub_addr;
            pub_addr.sin_family = AF_INET;
            pub_addr.sin_port = htons(PUB_PORT);
            inet_pton(AF_INET, PUB_IP, &pub_addr.sin_addr);

            if (connect(pub_sock, (struct sockaddr *)&pub_addr, sizeof(pub_addr)) == -1) {
                perror("connect");
                close(client_sock);
                close(pub_sock);
                exit(EXIT_FAILURE);
            }

            send(pub_sock, request, strlen(request), 0);

            char response[1024];
            ssize_t bytes_read = recv(pub_sock, response, 1024, 0);
            if (bytes_read == -1) {
                perror("recv");
                free(request);
                close(client_sock);
                close(pub_sock);
                exit(EXIT_FAILURE);
            }
            response[bytes_read] = '\0';
            printf("Risposta del pub: %s\n", response);

            int tavolo_assegnato;
            if (strncmp(response, "Sì, c'è il tavolo ", strlen("Sì, c'è il tavolo ")) == 0) {
                sscanf(response, "Sì, c'è il tavolo %d disponibile.", &tavolo_assegnato);

                // Invia il menù al cliente specificando il tavolo
                char menu[1024];
                snprintf(menu, sizeof(menu), "Il cameriere ti ha fatto accomodare al tavolo %d.\n"
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
                send(client_sock, menu, strlen(menu), 0);
                
                // Ricevi l'ordine completo dal cliente
                char order[1024];
                ssize_t order_len = recv(client_sock, order, 1024, 0);
                if (order_len == -1) {
                    perror("recv");
                    free(request);
                    close(client_sock);
                    close(pub_sock);
                    exit(EXIT_FAILURE);
                }
                order[order_len] = '\0';
                printf("Ordine ritirato dal tavolo %d: %s.\nLo invio al pub...\n", tavolo_assegnato, order);
                
                // Inoltra l'ordine al pub
                send(pub_sock, order, order_len, 0);
                
                // Ricevi la conferma dal pub che l'ordine è pronto
                char preparation_ack[1024];
                ssize_t ack_len = recv(pub_sock, preparation_ack, sizeof(preparation_ack), 0);
                if (ack_len == -1) {
                    perror("recv");
                    free(request);
                    close(client_sock);
                    close(pub_sock);
                    exit(EXIT_FAILURE);
                }
                preparation_ack[ack_len] = '\0';
                printf("Avviso dal pub: %s\nVado a servirlo al tavolo...\n", preparation_ack);
                
                // Invio la conferma al cliente
                send(client_sock, preparation_ack, strlen(preparation_ack), 0);
            } else {
                // Avvisa il cliente che non ci sono posti disponibili nel pub
                send(client_sock, response, strlen(response), 0);
            }

            // Attendiamo il messaggio relativo all'uscita del cliente dal pub
            char exit_message[1024];
            ssize_t exit_len = recv(client_sock, exit_message, sizeof(exit_message), 0);
            if (exit_len == -1) {
                perror("recv");
                free(request);
                close(client_sock);
                close(pub_sock);
                exit(EXIT_FAILURE);
            }
            exit_message[exit_len] = '\0';
            printf("Il cliente al tavolo %d ha lasciato il pub.\n", tavolo_assegnato);

            // Comunicalo al pub per fargli liberare il tavolo
            send(pub_sock, exit_message, strlen(exit_message), 0);

            free(request);
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