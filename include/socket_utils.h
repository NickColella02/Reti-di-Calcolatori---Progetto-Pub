#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H
#define PUB_IP "127.0.0.1"
#define PUB_PORT 8889
#define CAMERIERE_PORT 8888

int create_socket();

void set_socket_option(int sock);

void bind_socket(int sock, const char *ip, int port);

void listen_socket(int sock);

int accept_connection(int sock, struct sockaddr_in *client_addr);

int connect_to_address(const char *ip, int port);

void receive_message(int sock, char *message, size_t message_size);

void handle_terminate_generic(int sock, char *message);

void handle_terminate_for_cameriere(int pub_sock, int client_sock, char *message);

void handle_terminate_for_pub(int sock, char *message, int tavolo_liberato, SharedData *shared_data);

#endif /* SOCKET_UTILS_H */
