#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H
#define PUB_IP "127.0.0.1"
#define PUB_PORT 8889
#define CAMERIERE_PORT 8888

// Funzione per la creazione del socket
int create_socket();

// Funzione per il settaggio dell'opzione del socket
void set_socket_option(int sock);

// Funzione per il binding del socket all'indirizzo specificato
void bind_socket(int sock, const char *ip, int port);

// Funzione per l'ascolto sul socket
void listen_socket(int sock);

// Funzione per l'accettazione di una connessione sul socket
int accept_connection(int sock, struct sockaddr_in *client_addr);

// Funzione per la connessione a un indirizzo specificato
int connect_to_address(const char *ip, int port);

// Funzione per ricevere i messaggi dal socket con gestione degli errori
void receive_message(int sock, char *message, size_t message_size);

#endif /* SOCKET_UTILS_H */
