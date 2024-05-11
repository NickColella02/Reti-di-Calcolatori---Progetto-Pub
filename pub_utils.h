#ifndef PUB_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define PUB_UTILS_H
#define SHM_KEY 3456
#define NUMERO_TAVOLI 3
#define MESSAGE_SIZE 1024

typedef struct {
    int tavoli_occupati[NUMERO_TAVOLI]; // Array di flag per i tavoli occupati
} SharedData;

// Funzione per verificare se la stringa è un numero compreso tra min e max
int is_valid_number(const char *str, int min, int max);

void prepara_ordine(int tavolo_assegnato);

void libera_tavolo(int tavolo_liberato, SharedData *shared_data);

#endif /* PUB_UTILS_H */