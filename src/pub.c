#include "pub_utils.h"
#include "socket_utils.h"

int server_sock;
int client_sock;
SharedData *shared_data; // Puntatore alla struttura per la memoria condivisa

// Handler di pub.c per il segnale SIGINT
void sigint_handler(int sig) {
    // Invio un messaggio speciale al cameriere per indicare che il client sta terminando
    send(client_sock, "pub_terminate", strlen("pub_terminate"), 0);
    close(server_sock);
    close(client_sock);
    shmdt(shared_data);
    exit(EXIT_SUCCESS);
}

int main() {
    int shmid; // Identificatore della memoria condivisa
    key_t key = SHM_KEY; // Chiave per la memoria condivisa

    signal(SIGINT, sigint_handler);

    // Creazione/accesso alla memoria condivisa
    if ((shmid = shmget(key, sizeof(SharedData), IPC_CREAT | 0666)) < 0) {
        perror("Errore durante la shmget");
        exit(EXIT_FAILURE);
    }

    // Attacco della memoria condivisa
    if ((shared_data = (SharedData *)shmat(shmid, NULL, 0)) == (SharedData *)-1) {
        perror("Errore durante la shmat");
        exit(EXIT_FAILURE);
    }

    // Inizializzazione dei tavoli come liberi
    for (int i = 0; i < NUMERO_TAVOLI; i++) {
        shared_data->tavoli_occupati[i] = 0; // 0 indica che il tavolo è libero
    }

    // Creazione del socket
    server_sock = create_socket(); 
    set_socket_option(server_sock);
    bind_socket(server_sock, PUB_IP, PUB_PORT); 
    listen_socket(server_sock);

    printf("Il pub è operativo...\n\n");

    char message[MESSAGE_SIZE];

    while (1) {
        struct sockaddr_in client_addr;
        client_sock = accept_connection(server_sock, &client_addr);

        // Fork per gestire le comunicazioni coi camerieri
        pid_t pid = fork();
        if (pid == -1) {
            perror("Errore durante fork");
            close(client_sock);
            continue;
        }

        if (pid == 0) {
            // Processo figlio
            close(server_sock);

            memset(message, 0, MESSAGE_SIZE);

            // Ricevo la richiesta di posti disponibili dal cameriere
            receive_message(client_sock, message, MESSAGE_SIZE);
            handle_terminate_generic(client_sock, message);
            printf("Un cameriere chiede: %s\n", message);
            sleep(2);

            memset(message, 0, MESSAGE_SIZE);

            // Provo ad assegnare un tavolo 
            int tavolo_assegnato = assegna_tavolo(shared_data);

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
                
                // Ricevo l'ordine dal cameriere
                receive_message(client_sock, message, MESSAGE_SIZE);
                handle_terminate_for_pub(client_sock, message, tavolo_assegnato, shared_data);
                printf("C'è un ordine da preparare per il tavolo %d: %s.\n", tavolo_assegnato, message);
                sleep(2);
                
                // Preparo l'ordine
                prepara_ordine(tavolo_assegnato);
                
                memset(message, 0, MESSAGE_SIZE);
                
                // Dopo la preparazione dell'ordine, il pub lo avvisa il cameriere che è pronto
                snprintf(message, sizeof(message), "Ordine per il tavolo %d pronto al servizio.", tavolo_assegnato);
                send(client_sock, message, strlen(message), 0);
                
                memset(message, 0, MESSAGE_SIZE);

                // Processo di liberazione del tavolo
                receive_message(client_sock, message, MESSAGE_SIZE);
                handle_terminate_for_pub(client_sock, message, tavolo_assegnato, shared_data);
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
