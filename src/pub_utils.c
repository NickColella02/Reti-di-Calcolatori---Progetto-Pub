#include "pub_utils.h"

// Funzione per verificare se la portata inserita è corretta
int is_valid_number(const char *str, int min, int max) {
    int num = atoi(str);
    return num >= min && num <= max;
}

// Funzione per l'invio del menù al cliente
void invia_menu(int client_sock, char *message, int tavolo_assegnato) {
    // Costruzione del messaggio
    snprintf(message, MESSAGE_SIZE, "Il cameriere ti ha fatto accomodare al tavolo %d.\n"
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
    // Invio del messaggio al cliente
    send(client_sock, message, strlen(message), 0);
}

// Funzione per simulare la preparazione dell'ordine
void prepara_ordine(int tavolo_assegnato) {
    printf("Preparazione dell'ordine per il tavolo %d in corso...\n", tavolo_assegnato);
    sleep(3);
    printf("L'ordine per il tavolo %d è pronto per essere ritirato dal cameriere!\n", tavolo_assegnato);
}

// Funzione per assegnare un tavolo ad un cliente
int assegna_tavolo(SharedData *shared_data) {
    int tavolo_assegnato = -1; // Inizializziamo a -1 in caso non ci siano tavoli disponibili
    for (int i = 0; i < NUMERO_TAVOLI; i++) {
        if (shared_data->tavoli_occupati[i] == 0) { // Se il tavolo è libero
            tavolo_assegnato = i + 1; // Assegniamo il tavolo disponibile più basso
            shared_data->tavoli_occupati[i] = 1; // Impostiamo il tavolo come occupato
            break;
        }
    }
    return tavolo_assegnato;
}

// Funzione per liberare un tavolo che era occupato da un cliente
void libera_tavolo(int tavolo_liberato, SharedData *shared_data) {
    tavolo_liberato--;
    if (tavolo_liberato >= 0 && tavolo_liberato < NUMERO_TAVOLI) {
        shared_data->tavoli_occupati[tavolo_liberato] = 0; // Imposta il tavolo come libero
        printf("Il tavolo %d è stato liberato e ora è disponibile.\n", tavolo_liberato + 1);
    } else {
        printf("Tentativo di liberare un tavolo non valido.\n");
    }
}