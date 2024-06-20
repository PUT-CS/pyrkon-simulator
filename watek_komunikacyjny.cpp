#include "watek_komunikacyjny.hpp"
#include "main.hpp"
#include "util.hpp"

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void* startKomWatek(void* ptr)
{
    MPI_Status status;
    pthread_mutex_t ticketMut = PTHREAD_MUTEX_INITIALIZER;
    Packet packet;

    while (true) {
        MPI_Recv(&packet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        pthread_mutex_lock(&lamportMut);
        bool theirTicketPriority = false;
        bool theirWorkshopPriority = false;
        if (packet.ts < PYRKON_LAMPORT_CLOCKS[status.MPI_SOURCE])
            theirTicketPriority = true;
        if (packet.ts < WORKSHOP_LAMPORT_CLOCKS[status.MPI_SOURCE])
            theirWorkshopPriority = true;
        if (LAMPORT < packet.ts) {
            LAMPORT = packet.ts;
        }
        LAMPORT++;
        pthread_mutex_unlock(&lamportMut);
        switch ((Tag)status.MPI_TAG) {
        case TicketReq: {
            if (stan == WaitForPyrkon || (stan == TicketWait && theirTicketPriority) || stan == OffPyrkon) {
                sendPacket(0, status.MPI_SOURCE, TicketAck);
            } else {
                pthread_mutex_lock(&ticketMut);
                TICKET_QUEUE.push_back(status.MPI_SOURCE);
                pthread_mutex_unlock(&ticketMut);
            }
            break;
        }
        case TicketAck: {
            MY_TICKET_ACK_COUNT++;
            debug("Dostałem TICKET_ACK od %d, mam już %d", status.MPI_SOURCE, TicketAckNum);
            break;
        }
        case WorkshopReq: {
            debug("%d chce na warsztat %d.", status.MPI_SOURCE, pakiet.data);
            bool isAtThisWorkshop = (stan == OnWorkshop && packet.data == WORKSHOP_WISHLIST[SOUGHT_AFTER_IDX]);
            bool wantsThisWorkshop = (stan == WorkshopWait && packet.data == WORKSHOP_WISHLIST[SOUGHT_AFTER_IDX] && !theirWorkshopPriority);
            if (isAtThisWorkshop || wantsThisWorkshop) {
                pthread_mutex_lock(&ticketMut);
                WORKSHOP_QUEUE.push_back(status.MPI_SOURCE);
                pthread_mutex_unlock(&ticketMut);
            } else {
                sendPacket(0, status.MPI_SOURCE, WorkshopAck);
            }
            break;
        }
        case WorkshopAck: {
            MY_WORKSHOP_ACK_COUNT++;
            debug("Dostałem WORKSHOP_ACK od %d, mam teraz %d", status.MPI_SOURCE, WorkshopAckNum);
            break;
        }
        case PyrkonEndReq: {
            if (stan == OffPyrkon) {
                sendPacket(0, status.MPI_SOURCE, PyrkonEndYes);
            } else {
                sendPacket(0, status.MPI_SOURCE, PyrkonEndNo);
            }
            break;
        }
        case PyrkonEndYes:
            if (stan == OffPyrkon)
                MY_YES_END_RESPONSE_COUNT++;
        case PyrkonEndNo:
            if (stan == OffPyrkon)
                MY_END_RESPONSE_COUNT++;
            break;
        case Tag::StartPyrkon: {
            if (stan == OffPyrkon) {
                debug("Nowy Pyrkon!");
                NEW_PYRKON_P = true;
            }
            break;
        }
        default:
            break;
        }
    }
}
