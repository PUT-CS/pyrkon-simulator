#include "main.hpp"
#include "util.hpp"
MPI_Datatype MPI_PAKIET_T;

/* 
 * w util.h extern state_t stan (czyli zapowiedź, że gdzieś tam jest definicja
 * tutaj w util.c state_t stan (czyli faktyczna definicja)
 */
State stan=WaitForPyrkon;

/* zamek wokół zmiennej współdzielonej między wątkami. 
 * Zwróćcie uwagę, że każdy proces ma osobą pamięć, ale w ramach jednego
 * procesu wątki współdzielą zmienne - więc dostęp do nich powinien
 * być obwarowany muteksami
 */
pthread_mutex_t stateMut = PTHREAD_MUTEX_INITIALIZER;

// struct tagNames_t{
//     const char *name;
//     int tag;
// } tagNames[] = { { "prośba o bilet", TICKET_REQ }, { "potwierdzenie biletu", TICKET_ACK}, 
//                 { "prośba o warsztat", WORKSHOP_REQ}, {"potwierdzenie warsztatu", WORKSHOP_ACK}, {"zapytanie o koniec", END_REQ},
//                 { "potwierdzenie końca", END_YES}, {"zaprzeczenie końca", END_NO}, {"rozpoczęcie Pyrkonu", START_PYRKON} };

// const char *const tag2string( int tag )
// {
//     for (int i=0; i <sizeof(tagNames)/sizeof(struct tagNames_t);i++) {
// 	if ( tagNames[i].tag == tag )  return tagNames[i].name;
//     }
//     return "<unknown>";
// }
/* tworzy typ MPI_PAKIET_T
*/
void inicjuj_typ_pakietu()
{
    /* Stworzenie typu */
    /* Poniższe (aż do MPI_Type_commit) potrzebne tylko, jeżeli
       brzydzimy się czymś w rodzaju MPI_Send(&typ, sizeof(pakiet_t), MPI_BYTE....
    */
    /* sklejone z stackoverflow */
    int       blocklengths[NITEMS] = {1,1,1};
    MPI_Datatype typy[NITEMS] = {MPI_INT, MPI_INT, MPI_INT};

    MPI_Aint     offsets[NITEMS]; 
    offsets[0] = offsetof(Packet, ts);
    offsets[1] = offsetof(Packet, src);
    offsets[2] = offsetof(Packet, data);

    MPI_Type_create_struct(NITEMS, blocklengths, offsets, typy, &MPI_PAKIET_T);

    MPI_Type_commit(&MPI_PAKIET_T);
}

/* opis patrz util.h */
void sendPacket(Packet *pkt, int destination, int tag)
{
    pthread_mutex_t lamportMut = PTHREAD_MUTEX_INITIALIZER;
    int freepkt=0;
    if (pkt==0) { pkt = (Packet*)malloc(sizeof(Packet)); freepkt=1;}
    pkt->src = RANK;
    pthread_mutex_lock(&lamportMut);
    pkt->ts = ++LAMPORT;
    pthread_mutex_unlock(&lamportMut);
    MPI_Send( pkt, 1, MPI_PAKIET_T, destination, tag, MPI_COMM_WORLD);
    debug("Wysyłam %s do %d\n", tag2string( tag), destination);
    if (freepkt) free(pkt);
}

void changeState( State newState )
{
    pthread_mutex_lock( &stateMut );
//    if (stan==OffPyrkon) { 
//	  pthread_mutex_unlock( &stateMut );
//        return;
//    }
    stan = newState;
    pthread_mutex_unlock( &stateMut );
}
