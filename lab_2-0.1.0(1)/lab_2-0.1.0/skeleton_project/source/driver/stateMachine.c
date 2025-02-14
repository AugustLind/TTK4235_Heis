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
    printf("Elevator is in first floor\n");
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
            int index = j + i*4;
            if (elevio_callButton(j, i)){
                // printf("%d\n", elevio_callButton(j, i)); // Skriv ut heltallet som resultat av funksjonen
                // printf("%d\n", j); 
                if (index < 15){
                    state->queue[index] = j;
                }
                else {
                    printf("Queue is full\n");
                }
            }
            else {
                state->queue[index] = -1;
            }
        }
    }

}
    // for (int i = 0; i < 15; i++){
    //      printf("%d" , state->orders[i]);
         
    // }
void nextFloor(struct StateMachine *state) {
    //henter første verdi i køen 
    int next = 2;
    //sjekker hvilken etasje man er i og sammenligner
    //om man skal lengre opp eller lengre ned 
    int current = elevio_floorSensor();
    // for (int i = 0; i < 15; i++){
    //       printf("%d" , state->queue[i]);
         
    // }
    // printf("/n");
    //printf("%d",elevio_floorSensor());
    if (current < next) {
        state->direction = DIRN_UP;
    }
    if (current > next) {
        state->direction = DIRN_DOWN;
    }
    if (current == next) {
        state->direction = DIRN_STOP;
    }
    elevio_motorDirection(state->direction);
    printf("%d", elevio_obstruction());
    //åpne dør 
}

int compare (const void * a, const void * b){
    return ( *(int*)a - *(int*)b );
}

void sortQueue(struct StateMachine *state){
    int index = 0;
    
    for (int i = 0; i < 15; i++){
        if (state->queue[i] != -1){
            state->orders[index++] = state->queue[i];
        }    
    }
    qsort(state->orders, index, sizeof(int),compare);

    for (int i = 0; i < 15; i++){
          printf("%d" , state->orders[i]);
         
    }
}