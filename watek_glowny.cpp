#include "watek_glowny.hpp"
#include "main.hpp"
#include "util.hpp"
#include <cstdint>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <vector>

template <typename T>
bool vectorContains(std::vector<T> vec, T elem)
{
    for (int i = 0; i < vec.size(); ++i) {
        if (vec[i] == elem) {
            return true;
        }
    }
    return false;
}

void mainLoop()
{
    srandom(RANK);
    int tag;
    int percentage;
    Packet* pkt = new Packet();
    println("%d bilety, %d warsztaty z %d miejscami każdy\n", TICKET_COUNT, WORKSHOP_COUNT, WORKSHOP_MEMBER_COUNT);
    bool printFlag = true;

    while (true) {
        switch (stan) {
        case WaitForPyrkon: {
            for (int i = 0; i < SIZE; ++i) {
                PYRKON_LAMPORT_CLOCKS[i] = INT32_MAX;
            }
            for (int i = 0; i < SIZE; ++i) {
                WORKSHOP_LAMPORT_CLOCKS[i] = INT32_MAX;
            }
            printFlag = 1;
            if ((random() % 100) < 25) {
                TICKET_QUEUE = {};
                WORKSHOP_QUEUE = {};
                changeState(TicketWait);
                println("Ubiegam się o bilet na Pyrkon %d", PYRKON_EDITION)
                    debug("Zmieniam stan na wysyłanie");
                MY_TICKET_ACK_COUNT = 0;
                
                pthread_mutex_lock(&lamportMut);
                for (int i = 0; i <= SIZE - 1; i++) {
                    if (i != RANK) {
                        sendPacket(pkt, i, TicketReq);
                    }
                    PYRKON_LAMPORT_CLOCKS[i] = LAMPORT;
                }
                pthread_mutex_unlock(&lamportMut);
            }
            break;
        }
        case TicketWait: {
            println("Czekam na wejście na Pyrkon %d", PYRKON_EDITION);
            if (MY_TICKET_ACK_COUNT >= SIZE - TICKET_COUNT) {
                changeState(OnPyrkon);
                SOUGHT_AFTER_IDX = 0;
                int wishlistLength = std::max(1, rand() % WORKSHOP_COUNT);
                WORKSHOP_WISHLIST = std::vector<int>(wishlistLength);

                // make a unique random wishlist
                for (int i = 0; i < wishlistLength; ++i) {
                    int workshop;
                    do {
                        workshop = rand() % WORKSHOP_COUNT;
                    } while (vectorContains(WORKSHOP_WISHLIST, workshop));
                    WORKSHOP_WISHLIST[i] = workshop;
                }

                debug("Wylosowałem %d warsztatów:", wantedWorkshopCount);
                for (int i = 0; i < WORKSHOP_WISHLIST.size(); ++i) {
                    debug("Warsztat %d: %d", i + 1, WantedWorkshops[i]);
                }
            }
            break;
        }
        case OnPyrkon: {
            if (SOUGHT_AFTER_IDX == 0) {
                println("Wszedłem na Pyrkon %d! Szukam warsztatów", PYRKON_EDITION);
            } else {
                println("Wyszedłem z warsztatu, wróciłem na Pyrkon %d!", PYRKON_EDITION);
            }
            debug("Perc: %d", perc);
            if (SOUGHT_AFTER_IDX < WORKSHOP_WISHLIST.size()) {
                MY_WORKSHOP_ACK_COUNT = 0;
                pkt->data = WORKSHOP_WISHLIST[SOUGHT_AFTER_IDX];
                changeState(WorkshopWait);
                pthread_mutex_lock(&lamportMut);
                for (int i = 0; i <= SIZE - 1; ++i) {
                    if (i != RANK) {
                        sendPacket(pkt, i % SIZE, WorkshopReq);
                        WORKSHOP_LAMPORT_CLOCKS[i] = LAMPORT;
                    }
                }
                pthread_mutex_unlock(&lamportMut);
            } else {
                println("Wychodzę z Pyrkonu %d", PYRKON_EDITION);
                debug("W czasie Pyrkonu otrzymałem takie zgłoszenia:");
                for (int i = 0; i < TICKET_QUEUE.size(); ++i) {
                    debug("%d: %d. Wysyłam ACK", i + 1, TICKET_QUEUE[i]);
                    sendPacket(pkt, TICKET_QUEUE[i], Tag::TicketAck);
                }
                changeState(OffPyrkon);
                MY_END_RESPONSE_COUNT = 0;
                MY_YES_END_RESPONSE_COUNT = 0;
                for (int i = 0; i <= SIZE - 1; i++) {
                    if (i != RANK) {
                        sendPacket(pkt, i % SIZE, Tag::PyrkonEndReq);
                    }
                }
            }
            break;
        }
        case WorkshopWait: {
            println("Czekam na wejście na %d z kolei Warsztat %d", SOUGHT_AFTER_IDX, WORKSHOP_WISHLIST[SOUGHT_AFTER_IDX]);
            if (MY_WORKSHOP_ACK_COUNT >= SIZE - WORKSHOP_MEMBER_COUNT) {
                println("Wchodzę na warsztat %d", WORKSHOP_WISHLIST[SOUGHT_AFTER_IDX]);
                changeState(OnWorkshop);
            }
            break;
        }
        case OnWorkshop: {
            sleep(1);
            println("Wychodzę z warsztatu %d", WORKSHOP_WISHLIST[SOUGHT_AFTER_IDX]);
            debug("W czasie warsztatu otrzymałem takie zgłoszenia:");
            for (int i = 0; i < WORKSHOP_QUEUE.size(); ++i) {
                debug("%d: %d. Wysyłam ACK.", i + 1, WORKSHOP_QUEUE[i]);
                sendPacket(pkt, WORKSHOP_QUEUE[i], Tag::WorkshopAck);
            }
            WORKSHOP_QUEUE = std::vector<int>(SIZE);
            for (int i = 0; i < SIZE; ++i) {
                WORKSHOP_LAMPORT_CLOCKS[i] = 2147483647;
            }
            changeState(OnPyrkon);
            SOUGHT_AFTER_IDX++;
            break;
        }
        case OffPyrkon: {
            if (MY_END_RESPONSE_COUNT >= SIZE - 1) {
                if (MY_YES_END_RESPONSE_COUNT >= SIZE - 1) {
                    println("Jako ostatni wychodzę w %d. Zaczynam nowy Pyrkon!", PYRKON_EDITION);
                    if (NEW_PYRKON_P) {
                        println("Ktoś mnie uprzedził.");
                    } else {
                        for (int i = 0; i <= SIZE - 1; i++) {
                            if (i != RANK)
                                sendPacket(pkt, i % SIZE, Tag::StartPyrkon);
                        }
                    }
                    NEW_PYRKON_P = 0;
                    PYRKON_EDITION++;
                    changeState(WaitForPyrkon);
                } else {
                    if (printFlag) {
                        println("Nie wyszedłem ostatni z %d", PYRKON_EDITION);
                    }
                    if (NEW_PYRKON_P) {
                        println("Widzę, że ogłoszono nowy Pyrkon!");
                        NEW_PYRKON_P = 0;
                        ++PYRKON_EDITION;
                        changeState(WaitForPyrkon);
                    } else {
                        if (printFlag) {
                            println("Dalej czekam na kolejny Pyrkon");
                            printFlag = 0;
                        }
                    }
                }
            }
            break;
        }
        default:
            break;
        }
        sleep(SEC_IN_STATE);
    }
}
