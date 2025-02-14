#include "stateMachine.h"
#include "elevio.h"

#include <stdio.h>
#include <stdlib.h>

void getToFirstFloor(){
    //kjører ned til første etasje
    elevio_motorDirection(DIRN_DOWN);
    while(elevio_floorSensor() != 0){
        //venter til vi er i første etasje
    }
    elevio_motorDirection(DIRN_STOP);
    elevio_doorOpenLamp(1);
    //åpner døra
    //venter i 3 sekunder
    //lukker døra
    elevio_doorOpenLamp(0);
}

void init(){
    //vite hvilken etasje
    //tomme eventuell liste av bestillinger
    //kanskje sjekk
    //gjøre seg klar til bestillinger/arbeid
    elevio_init();
    getToFirstFloor();
}

void getOrders(struct StateMachine *state){
    //tar imot bestillinger
    //sjekker om bestillingen er gyldig
    //legger til bestillingen i køen
    //sorterer køen

    //ytterløkke for knappetyper
    //innerløkke for etasjer
    for (ButtonType i = BUTTON_HALL_UP; i <= BUTTON_CAB; i++){
        for (int j = 0; j < N_FLOORS; j++){
            if (elevio_callButton(j, i)){
                printf("%d\n", elevio_callButton(j, i)); // Skriv ut heltallet som resultat av funksjonen
                printf("%d\n", j); 
                int index = j + i*4;
                if (index < 15){
                    state->queue[index] = j;
                }
                else {
                    printf("Queue is full\n");
                }
            }
        }
    }

}

void nextFloor(struct StateMachine *state) {
    //henter første verdi i køen 
    int *next = state->queue[0];
    //sjekker hvilken etasje man er i og sammenligner
    //om man skal lengre opp eller lengre ned 
    int current = elevio_floorSensor();
    if (current + 1 < *next) {
        elevio_motorDirection(DIRN_UP);
        if (current + 1 == *next) {
            elevio_motorDirection(DIRN_STOP);
        }
    }
    if (current + 1 > *next) {
        elevio_motorDirection(DIRN_DOWN);
        if (current + 1 == *next) {
            elevio_motorDirection(DIRN_STOP);
        }
    }
    //åpne dør 
}


