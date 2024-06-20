#ifndef MAINH
#define MAINH
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <vector>

#include "util.hpp"

#define SEC_IN_STATE 1

#define ROOT 0

extern int RANK;
extern int SIZE;
extern int MY_ACK_COUNT;
extern pthread_t COMMS_THREAD;
extern int LAMPORT;
extern int PYRKON_EDITION;

//liczba biletów , warsztatów i miejsc na każdmy
extern int TICKET_COUNT;
extern int WORKSHOP_COUNT;
extern int WORKSHOP_MEMBER_COUNT;

// oczekujący na bilety i warsztaty
extern std::vector<int> TICKET_QUEUE;
extern std::vector<int> WORKSHOP_QUEUE;

// warsztaty do odwiedzenia
extern std::vector<int> WORKSHOP_WISHLIST;

// indeks aktualnie poszukiwanego warsztatu
extern int SOUGHT_AFTER_IDX;

extern int MY_TICKET_ACK_COUNT; // ile TICKET_ACK
extern int MY_WORKSHOP_ACK_COUNT; // ile WORKSHOP_ACK

extern int MY_YES_END_RESPONSE_COUNT; // ile END_YES
extern int MY_END_RESPONSE_COUNT; // ile END_YES lub END_NO

// tworzymy nowy?
extern int NEW_PYRKON_P;

extern pthread_mutex_t lamportMut;
// wartości lamporta w momencie wysyłania żądania dostępu do Pyrkonu do danego procesu
extern std::vector<int> PYRKON_LAMPORT_CLOCKS;
extern std::vector<int> WORKSHOP_LAMPORT_CLOCKS;

#ifdef DEBUG
#define debug(FORMAT,...) printf("%c[%d;%dm [%d][LAMPORT=%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(RANK/7))%2, 31+(6+RANK)%7, RANK, LAMPORT, ##__VA_ARGS__, 27,0,37);
#else
#define debug(...) ;
#endif

// makro println - to samo co debug, ale wyświetla się zawsze
#define println(FORMAT,...) printf("%c[%d;%dm [%d][LAMPORT=%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(RANK/7))%2, 31+(6+RANK)%7, RANK, LAMPORT, ##__VA_ARGS__, 27,0,37);


#endif
