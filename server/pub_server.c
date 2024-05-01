#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define MAX_TABLES 10
#define MENU_SIZE 3

// Struct per memorizzare gli ordini
struct Order {
    char **items; // Array di puntatori a stringhe per gli articoli
    int table_number;
    int num_items; // Numero attuale di elementi nell'ordine
};

int main() {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *welcome_message = "Benvenuto al Pub!\n";
    char *menu[MENU_SIZE] = {"Birra", "Vino", "Cocktail"};
    struct Order orders[MAX_TABLES];
    int num_tables_available = MAX_TABLES;

    // Creazione del socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Errore nella creazione del socket");
        exit(EXIT_FAILURE);
    }

    // Impostazione delle opzioni del socket
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Errore nell'impostazione delle opzioni del socket");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind del socket all'indirizzo e alla porta
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("Errore durante il bind del socket");
        exit(EXIT_FAILURE);
    }

    // Ascolto delle connessioni in ingresso
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Inizializzazione degli ordini
    for (int i = 0; i < MAX_TABLES; i++) {
        orders[i].items = malloc(sizeof(char *) * MENU_SIZE); // Alloca memoria per gli elementi dell'ordine
        if (orders[i].items == NULL) {
            perror("Errore nell'allocazione di memoria per gli elementi dell'ordine");
            exit(EXIT_FAILURE);
        }
        orders[i].num_items = 0; // Inizializza il numero di elementi a 0
    }

    // Ciclo di accettazione delle connessioni in ingresso e gestione degli ordini
    while(1) {
        printf("Il pub attende clienti...\n");

        // Accettazione delle connessioni in ingresso
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("Errore durante l'accettazione di una connessione");
            exit(EXIT_FAILURE);
        }

        // Invia un messaggio di benvenuto al client
        send(new_socket, welcome_message, strlen(welcome_message), 0);

        // Verifica se il cliente può entrare nel pub
        if (num_tables_available > 0) {
            num_tables_available--;

            char table_message[1024];
            sprintf(table_message, "Ti sei accomodato al tavolo numero %d.\n", MAX_TABLES - num_tables_available);
            send(new_socket, table_message, strlen(table_message), 0);

            // Invia il menu al cliente
            char menu_message[1024];
            sprintf(menu_message, "Menu':\n1. %s\n2. %s\n3. %s\n", menu[0], menu[1], menu[2]);
            send(new_socket, menu_message, strlen(menu_message), 0);
            
            // Registrazione dell'ordine relativo al tavolo in cui si è accomodato il client
            int current_table = MAX_TABLES - num_tables_available - 1;
            int order_index = 0;

            while (1) {
                // Ricevi la selezione del cliente
                valread = read(new_socket, buffer, 1024);

                // Se la selezione è '0', prendi l'ordine e consegnalo al pub
                if (strcmp(buffer, "0\n") == 0) {
                    printf("Ricevuto un ordine dal tavolo numero %d, lo sto consegnando al pub.\n", current_table + 1);
                    break;
                }

                // Alloca memoria per la nuova selezione e copia il nome del menu
                orders[current_table].items[order_index] = strdup(menu[atoi(buffer) - 1]);
                if (orders[current_table].items[order_index] == NULL) {
                    perror("Errore nell'allocazione di memoria per un elemento nell'ordine");
                    exit(EXIT_FAILURE);
                }
                order_index++;
                orders[current_table].num_items++;
            }

            // Conferma al cliente che l'ordine è stato consegnato al cameriere
            char confirmation_message[1024];
            sprintf(confirmation_message, "Hai consegnato il tuo ordine al cameriere, attendi che arrivi al tavolo.\n");
            send(new_socket, confirmation_message, strlen(confirmation_message), 0);

            // Resetta l'ordine corrente
            for (int i = 0; i < orders[current_table].num_items; i++) {
                free(orders[current_table].items[i]);
                orders[current_table].items[i] = NULL;
            }
            orders[current_table].num_items = 0;
        } else {
            // Avvisa il cliente che non ci sono posti disponibili nel pub
            char no_seats_message[] = "Spiacente, il pub è pieno al momento. Riprova più tardi.\n";
            send(new_socket, no_seats_message, strlen(no_seats_message), 0);
        }

        // Chiudi la connessione con il client
        close(new_socket);
    }
    
    return 0;
}