/* w main.h także makra println oraz debug -  z kolorkami! */
#include "main.hpp"
#include "watek_glowny.hpp"
#include "watek_komunikacyjny.hpp"
#include <cstdint>
#include <vector>
#define DEBUG

int RANK, SIZE;
int MY_ACK_COUNT = 0;
int LAMPORT = 0;
int TICKET_COUNT, WORKSHOP_COUNT, WORKSHOP_MEMBER_COUNT;

std::vector<int> TICKET_QUEUE;
std::vector<int> WORKSHOP_QUEUE;
std::vector<int> WORKSHOP_WISHLIST;

int MY_TICKET_ACK_COUNT, MY_WORKSHOP_ACK_COUNT;
int TICKET_QUEUE_LENGTH, WORKSHOP_QUEUE_LENGTH;
int MY_YES_END_RESPONSE_COUNT, MY_END_RESPONSE_COUNT;
int NEW_PYRKON_P = 0;
int PYRKON_EDITION = 2024;
int WORKSHOP_WISHLIST_LENGTH = 0;
int SOUGHT_AFTER_IDX = -1;
std::vector<int> PYRKON_LAMPORT_CLOCKS;
std::vector<int> WORKSHOP_LAMPORT_CLOCKS;
pthread_mutex_t lamportMut = PTHREAD_MUTEX_INITIALIZER;

pthread_t COMMS_THREAD;

void finalizuj()
{
    pthread_mutex_destroy( &stateMut);
    /* Czekamy, aż wątek potomny się zakończy */
    println("czekam na wątek \"komunikacyjny\"\n" );
    pthread_join(COMMS_THREAD,NULL);
    MPI_Type_free(&MPI_PAKIET_T);
    MPI_Finalize();
}

void check_thread_support(int provided)
{
    printf("THREAD SUPPORT: chcemy %d. Co otrzymamy?\n", provided);
    switch (provided) {
        case MPI_THREAD_SINGLE: 
            printf("Brak wsparcia dla wątków, kończę\n");
            /* Nie ma co, trzeba wychodzić */
	    fprintf(stderr, "Brak wystarczającego wsparcia dla wątków - wychodzę!\n");
	    MPI_Finalize();
	    exit(-1);
	    break;
        case MPI_THREAD_FUNNELED: 
            printf("tylko te wątki, ktore wykonaly mpi_init_thread mogą wykonać wołania do biblioteki mpi\n");
	    break;
        case MPI_THREAD_SERIALIZED: 
            /* Potrzebne zamki wokół wywołań biblioteki MPI */
            printf("tylko jeden watek naraz może wykonać wołania do biblioteki MPI\n");
	    break;
        case MPI_THREAD_MULTIPLE: printf("Pełne wsparcie dla wątków\n"); /* tego chcemy. Wszystkie inne powodują problemy */
	    break;
        default: printf("Nikt nic nie wie\n");
    }
}


int main(int argc, char **argv)
{
    if(argc < 3){
        printf("Za malo argumentow!\n");
        return 0;
    }
    TICKET_COUNT = atoi(argv[1]);
    WORKSHOP_COUNT = atoi(argv[2]);
    WORKSHOP_MEMBER_COUNT = atoi(argv[3]);
    MPI_Status status;
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    check_thread_support(provided);
    srand(RANK);
    inicjuj_typ_pakietu();
    MPI_Comm_size(MPI_COMM_WORLD, &SIZE);
    MPI_Comm_rank(MPI_COMM_WORLD, &RANK);
    PYRKON_LAMPORT_CLOCKS = std::vector<int>(SIZE);
    WORKSHOP_LAMPORT_CLOCKS = std::vector<int>(SIZE);

    for(int i = 0; i < SIZE; ++i){
        PYRKON_LAMPORT_CLOCKS[i] = INT32_MAX;
    }
    for(int i = 0; i < SIZE; ++i){
        WORKSHOP_LAMPORT_CLOCKS[i] = INT32_MAX;
    }
    pthread_create( &COMMS_THREAD, NULL, startKomWatek , 0);
    mainLoop(); 
    finalizuj();
    return 0;
}

