#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "pub_utils.h"
#include "socket_utils.h"

#define PUB_IP "127.0.0.1"
#define PUB_PORT 8889

void prepara_ordine(int tavolo_assegnato) {
    // Simulazione di preparazione dell'ordine
    printf("Preparazione dell'ordine per il tavolo %d in corso...\n", tavolo_assegnato);
    sleep(3);
    printf("L'ordine per il tavolo %d è pronto per essere ritirato dal cameriere!\n", tavolo_assegnato);
}

void libera_tavolo(int tavolo_liberato, SharedData *shared_data) {
    tavolo_liberato--;
    if (tavolo_liberato >= 0 && tavolo_liberato < NUMERO_TAVOLI) {
        shared_data->tavoli_occupati[tavolo_liberato] = 0; // Imposta il tavolo come libero
        printf("Il tavolo %d è stato liberato e ora è disponibile.\n", tavolo_liberato + 1);
    } else {
        printf("Tentativo di liberare un tavolo non valido.\n");
    }
}

int main() {
    int shmid;
    key_t key = SHM_KEY;
    SharedData *shared_data;

    // Creazione/accesso alla memoria condivisa
    if ((shmid = shmget(key, sizeof(SharedData), IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Attacco della memoria condivisa
    if ((shared_data = (SharedData *)shmat(shmid, NULL, 0)) == (SharedData *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    // Inizializzazione dei tavoli come liberi
    for (int i = 0; i < NUMERO_TAVOLI; i++) {
        shared_data->tavoli_occupati[i] = 0; // 0 indica che il tavolo è libero
    }

    // Creazione del socket del server
    int server_sock = create_socket();
    set_socket_option(server_sock);
    bind_socket(server_sock, PUB_IP, PUB_PORT);
    listen_socket(server_sock);

    printf("Il pub è operativo...\n\n");

    while (1) {
        struct sockaddr_in client_addr;
        int client_sock = accept_connection(server_sock, &client_addr);

        // Fork per gestire la comunicazione con il client
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            close(client_sock);
            continue;
        }

        if (pid == 0) {
            // Processo figlio
            close(server_sock);

            // Riceve la richiesta dal cameriere
            char request[1024];
            ssize_t bytes_received = recv(client_sock, request, sizeof(request), 0);
            if (bytes_received == -1) {
                perror("recv");
                close(client_sock);
                exit(EXIT_FAILURE);
            }
            request[bytes_received] = '\0';
            printf("Un cameriere chiede: %s\n", request);

            // Controllo se ci sono tavoli disponibili
            char response[1024];
            int tavolo_assegnato = -1; // Inizializziamo a -1 in caso non ci siano tavoli disponibili
            for (int i = 0; i < NUMERO_TAVOLI; i++) {
                if (shared_data->tavoli_occupati[i] == 0) { // Se il tavolo è libero
                    tavolo_assegnato = i + 1; // Assegniamo il tavolo disponibile più basso
                    shared_data->tavoli_occupati[i] = 1; // Impostiamo il tavolo come occupato
                    break;
                }
            }

            if (tavolo_assegnato != -1) { // Se è stato assegnato un tavolo
                // Comunico al cameriere che c'è un tavolo disponibile e quale è stato assegnato
                snprintf(response, sizeof(response), "Sì, c'è il tavolo %d disponibile.", tavolo_assegnato);
            } else {
                // Comunico al cameriere che non ci sono posti disponibili
                strcpy(response, "Mi dispiace, non ci sono tavoli disponibili.");
            }

            // Invia la risposta al cameriere
            send(client_sock, response, strlen(response), 0);

            // Riceve l'ordine dal cameriere
            char order[1024];
            ssize_t order_len = recv(client_sock, order, sizeof(order), 0);
            if (order_len == -1) {
                perror("recv");
                close(client_sock);
                exit(EXIT_FAILURE);
            }
            order[order_len] = '\0';
            printf("C'è un ordine da preparare per il tavolo %d: %s.\n", tavolo_assegnato, order);

            // Prepara l'ordine
            prepara_ordine(tavolo_assegnato);

            // Dopo la preparazione dell'ordine, il pub lo consegna al cameriere per servirlo al tavolo
            char preparation_ack[1024];
            snprintf(preparation_ack, sizeof(preparation_ack), "Ordine per il tavolo %d pronto al servizio.", tavolo_assegnato);
            send(client_sock, preparation_ack, strlen(preparation_ack), 0);

            // Processo di liberazione del tavolo
            char exit_message[1024];
            ssize_t exit_len = recv(client_sock, exit_message, sizeof(exit_message), 0);
            if (exit_len == -1) {
                perror("recv");
                close(client_sock);
                exit(EXIT_FAILURE);
            }
            exit_message[exit_len] = '\0';
            printf("Il cliente al tavolo %d ha lasciato il pub.\n", tavolo_assegnato);
            libera_tavolo(tavolo_assegnato, shared_data);

            close(client_sock);
            exit(EXIT_SUCCESS);
        } else {
            // Processo padre
            close(client_sock);
        }
    }

    // Dettach della memoria condivisa
    shmdt(shared_data);

    close(server_sock);
    return 0;
}
