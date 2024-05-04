#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define CAMERIERE_PORT 8081
#define SERVER_IP "127.0.0.1"

int main() {
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    char *client_message = "Chiedo al cameriere se ci sono posti disponibili...";

    int sockfd;

    printf("Benvenuto al pub!\n");

    // Creazione del socket del cliente
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Errore nella creazione del socket");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(CAMERIERE_PORT);

    // Converti l'indirizzo IP da stringa a binario e assegnalo a sin_addr
    if(inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr)<=0) {
        perror("Indirizzo non valido/non supportato");
        exit(EXIT_FAILURE);
    }

    // Connessione al cameriere
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Errore durante la connessione al cameriere");
        exit(EXIT_FAILURE);
    }

    // Richiesta al cameriere se ci sono posti disponibili
    printf("%s\n", client_message);

    // Ricezione della risposta dal cameriere
    char read_buffer[1024] = {0};
    char write_buffer[1024] = {0};
    memset(read_buffer, 0, sizeof(read_buffer));
    memset(write_buffer, 0, sizeof(write_buffer));
    read(sockfd, read_buffer, 1024);
    strcpy(buffer, read_buffer);

    // Se la risposta è affermativa, ci accomodiamo e ci viene servito il menù
    if (strcmp(buffer, "Posto disponibile") == 0) {
        printf("Il cameriere ti ha fatto accomodare al tavolo.\n");
        printf("Ecco il menu':\n");
        printf("1. Burger\n");
        printf("2. Nonna assunta\n");
        printf("3. Hadoken\n");
        printf("4. Django\n");
        printf("5. Los Pollos\n");
        printf("6. Smashed One\n");
        printf("7. Smashed One Doppio Bacon\n");
        printf("8. Smashed One Extr3me\n");
        printf("9. Diego Armando Masardona\n");
        printf("10. Spring Mania Veg\n");
        printf("11. Genovese Astrospaziale\n");
        printf("12. Nennella MOP\n");

        // Selezione degli ordini
        printf("Seleziona i piatti inserendo i numeri corrispondenti separati da uno spazio (es. 1 3 5): ");
        fgets(buffer, 1024, stdin);

        // Invio dell'ordine al cameriere
        memset(write_buffer, 0, sizeof(write_buffer));
        strcpy(write_buffer, buffer);
        send(sockfd, write_buffer, strlen(buffer), 0);
    } else {
        printf("Mi dispiace, il pub è pieno.\n");
    }

    close(sockfd);
    return 0;
}
