#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

// Funzione per la creazione del socket
int create_socket() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Errore nella creazione del socket");
        exit(EXIT_FAILURE);
    }
    return sock;
}

// Funzione per il settaggio dell'opzione del socket
void set_socket_option(int sock) {
    int enable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("Errore nel settaggio dell'opzione del socket");
        exit(EXIT_FAILURE);
    }
}

// Funzione per il binding del socket all'indirizzo specificato
void bind_socket(int sock, const char *ip, int port) {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("Errore durante il bind");
        exit(EXIT_FAILURE);
    }
}

// Funzione per l'ascolto sul socket
void listen_socket(int sock) {
    if (listen(sock, 5) == -1) {
        perror("Errore durante l'ascolto");
        exit(EXIT_FAILURE);
    }
}

// Funzione per l'accettazione di una connessione sul socket
int accept_connection(int sock, struct sockaddr_in *client_addr) {
    int client_sock;
    int client_len = sizeof(*client_addr);
    if ((client_sock = accept(sock, (struct sockaddr *)client_addr, (socklen_t *)&client_len)) == -1) {
        perror("Errore durante l'accettazione");
        exit(EXIT_FAILURE);
    }
    return client_sock;
}

// Funzione per la connessione a un indirizzo specificato
int connect_to_address(const char *ip, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Errore nella creazione del socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("Errore durante la connessione");
        exit(EXIT_FAILURE);
    }

    return sock;
}

// Funzione per ricevere i messaggi dal socket con gestione degli errori
void receive_message(int sock, char *message, size_t message_size) {
    ssize_t bytes_received = recv(sock, message, message_size, 0);
    if (bytes_received == -1) {
        perror("recv");
        close(sock);
        exit(EXIT_FAILURE);
    }
    message[bytes_received] = '\0';
}