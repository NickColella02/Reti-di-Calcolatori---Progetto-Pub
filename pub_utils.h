#ifndef PUB_UTILS_H
#define PUB_UTILS_H

#define SHM_KEY 3456
#define NUMERO_TAVOLI 3

typedef struct {
    int tavoli_occupati[NUMERO_TAVOLI]; // Array di flag per i tavoli occupati
} SharedData;

#endif /* PUB_UTILS_H */