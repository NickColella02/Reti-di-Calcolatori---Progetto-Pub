#include "pub_utils.h"
#include "socket_utils.h"

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

    char message[MESSAGE_SIZE];

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

            memset(message, 0, MESSAGE_SIZE);

            // Riceve la richiesta dal cameriere
            receive_message(client_sock, message, MESSAGE_SIZE);
            printf("Un cameriere chiede: %s\n", message);
            sleep(2);

            memset(message, 0, MESSAGE_SIZE);

            int tavolo_assegnato = -1; // Inizializziamo a -1 in caso non ci siano tavoli disponibili
            for (int i = 0; i < NUMERO_TAVOLI; i++) {
                if (shared_data->tavoli_occupati[i] == 0) { // Se il tavolo è libero
                    tavolo_assegnato = i + 1; // Assegniamo il tavolo disponibile più basso
                    shared_data->tavoli_occupati[i] = 1; // Impostiamo il tavolo come occupato
                    break;
                }
            }

            if (tavolo_assegnato == -1) { // Se non è stato assegnato un tavolo
                // Comunico al cameriere che non ci sono posti disponibili
                strcpy(message, "Mi dispiace, non ci sono tavoli disponibili.");
                send(client_sock, message, strlen(message), 0);
            } else {
                // Comunico al cameriere che c'è un tavolo disponibile e quale è stato assegnato
                snprintf(message, sizeof(message), "Sì, c'è il tavolo %d disponibile.\n", tavolo_assegnato);
                printf("%s", message);
                fflush(stdout);
                
                // Invia la risposta al cameriere
                send(client_sock, message, strlen(message), 0);
                
                memset(message, 0, MESSAGE_SIZE);
                
                receive_message(client_sock, message, MESSAGE_SIZE);
                printf("C'è un ordine da preparare per il tavolo %d: %s.\n", tavolo_assegnato, message);
                sleep(2);
                
                // Prepara l'ordine
                prepara_ordine(tavolo_assegnato);
                
                // Dopo la preparazione dell'ordine, il pub lo consegna al cameriere per servirlo al tavolo
                memset(message, 0, MESSAGE_SIZE);
                snprintf(message, sizeof(message), "Ordine per il tavolo %d pronto al servizio.", tavolo_assegnato);
                send(client_sock, message, strlen(message), 0);
                
                // Processo di liberazione del tavolo
                memset(message, 0, MESSAGE_SIZE);
                receive_message(client_sock, message, MESSAGE_SIZE);
                libera_tavolo(tavolo_assegnato, shared_data);
            } 
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
