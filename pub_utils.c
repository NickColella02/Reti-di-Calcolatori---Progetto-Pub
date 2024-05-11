#include "pub_utils.h"

// Funzione per verificare se la stringa è un numero compreso tra min e max
int is_valid_number(const char *str, int min, int max) {
    int num = atoi(str);
    return num >= min && num <= max;
}

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