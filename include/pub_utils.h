#ifndef PUB_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>

#define PUB_UTILS_H
#define SHM_KEY 6789
#define NUMERO_TAVOLI 5
#define MESSAGE_SIZE 1024

typedef struct {
    int tavoli_occupati[NUMERO_TAVOLI]; // Array di flag per i tavoli occupati
} SharedData;

int is_valid_number(const char *str, int min, int max);

void invia_menu(int client_sock, char *message, int tavolo_assegnato);

void prepara_ordine(int tavolo_assegnato);

int assegna_tavolo(SharedData *shared_data);

void libera_tavolo(int tavolo_liberato, SharedData *shared_data);

#endif /* PUB_UTILS_H */