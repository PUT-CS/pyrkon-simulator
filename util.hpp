#ifndef UTILH
#define UTILH
#include "main.hpp"

/* typ pakietu */
typedef struct {
    int ts;       /* timestamp (zegar lamporta */
    int src;  
    int data;     /* przykładowe pole z danymi; można zmienić nazwę na bardziej pasującą */
} Packet;
/* packet_t ma trzy pola, więc NITEMS=3. Wykorzystane w inicjuj_typ_pakietu */
#define NITEMS 3

/* Typy wiadomości */
/* TYPY PAKIETÓW */
enum Tag {
    TicketReq = 1,
    TicketAck = 2,
    WorkshopReq = 3,
    WorkshopAck = 4,
    PyrkonEndReq = 5,
    PyrkonEndYes = 6,
    PyrkonEndNo = 7,
    StartPyrkon = 8
};

extern MPI_Datatype MPI_PAKIET_T;
void inicjuj_typ_pakietu();

/* wysyłanie pakietu, skrót: wskaźnik do pakietu (0 oznacza stwórz pusty pakiet), do kogo, z jakim typem */
void sendPacket(Packet *pkt, int destination, int tag);

typedef enum {WaitForPyrkon, TicketWait, OnPyrkon, WorkshopWait, OnWorkshop, OffPyrkon} State;
extern State stan;
extern pthread_mutex_t stateMut;
extern pthread_mutex_t lamportMut;

void changeState( State );
#endif
