#include "stateMachine.h"
#include "elevio.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // for sleep()

void getToFirstFloor(){
    elevio_motorDirection(DIRN_DOWN);
    while(elevio_floorSensor() != 0) {
    
    }
    elevio_motorDirection(DIRN_STOP);
    openDoor();
}

void init(struct StateMachine *state){
    getToFirstFloor();
    state->currentFloor = elevio_floorSensor();
    state->direction = DIRN_STOP;
    printf("Elevator is in first floor\n");
    state->active = 1;
}

void initQueue(struct StateMachine *state) {
    for (int i = 0; i < MAX_ORDERS; i++){
        state->queue[i] = -1;
        //state->queueDirection[i] = BUTTON_CAB;
    }
    state->orderCount = 0;
}

void addOrder(struct StateMachine *state, int floor, ButtonType btn) {
    for (int i = 0; i < state->orderCount; i++){
        if (state->queue[i] == floor)
            return;
    }
    if (state->orderCount < MAX_ORDERS){
        state->queue[state->orderCount] = floor;
        //state->queueDirection[state->orderCount] = btn;
        state->orderCount++;
        printf("La til bestilling for etasje %d\n", floor + 1);
    } else {
        printf("Køen er full\n");
    }
}

void removeOrder(struct StateMachine *state, int floor) {
    int index = -1;
    for (int i = 0; i < state->orderCount; i++){
        if (state->queue[i] == floor){
            index = i;
            break;
        }
    }
    if (index != -1){
        for (int i = index; i < state->orderCount - 1; i++){
            state->queue[i] = state->queue[i+1];
        }
        elevio_buttonLamp(floor, BUTTON_HALL_UP, 0);
        elevio_buttonLamp(floor, BUTTON_HALL_DOWN, 0);
        elevio_buttonLamp(floor, BUTTON_CAB, 0);
        state->queue[state->orderCount] = -1;
        //state->queueDirection[state->orderCount] = BUTTON_CAB;
        state->orderCount--;
        printf("Fjernet bestilling for etasje %d\n", floor + 1);
    }
}

int getNextOrder(struct StateMachine *state) {
    if (state->orderCount == 0)
        return state->currentFloor;
    int nextFloor = state->queue[0];
    for (int i = 0; i < state->orderCount; i++) {
        int floor = state->queue[i];
        //ButtonType btn = state->queueDirection[i];
        
        if (state->direction == DIRN_UP && floor > state->currentFloor) {
            if (floor < nextFloor || nextFloor < state->currentFloor) {
                nextFloor = floor;
            } 
        } else if (state->direction == DIRN_DOWN && floor < state->currentFloor) {
            if (floor > nextFloor || nextFloor > state->currentFloor) {
                nextFloor = floor;
            }
        } else if (state->direction == DIRN_STOP) {
            if (abs(floor - state->currentFloor) < abs(nextFloor - state->currentFloor)) {
                nextFloor = floor;
            }
        }
    }
    return nextFloor;
}

void getOrders(struct StateMachine *state) {
    for (ButtonType btn = BUTTON_HALL_UP; btn <= BUTTON_CAB; btn++){
        for (int floor = 0; floor < N_FLOORS; floor++){
            if (elevio_callButton(floor, btn)) {
                addOrder(state, floor, btn);
                elevio_buttonLamp(floor, btn, 1);
            }
        }
    }
}


void nextFloor(struct StateMachine *state) {
    int next = getNextOrder(state);
    int current = elevio_floorSensor();
    if (current != -1){
        state->currentFloor = current;
    }
    floorIndicator(state);

    if (state->currentFloor < next) {
        state->direction = DIRN_UP;
    } else if (state->currentFloor > next) {
        state->direction = DIRN_DOWN;
    } else {
        state->direction = DIRN_STOP;
        elevio_motorDirection(state->direction);
        if (state->orderCount > 0) {
            openDoor();
        }
        removeOrder(state, next);
        return;
    }
    elevio_motorDirection(state->direction);
    printf("Heisen beveger seg %s mot etasje %d. Current floor: %d\n", 
        (state->direction == DIRN_UP) ? "opp" : "ned", next + 1, current + 1);

}

void openDoor() {
     //åpner og lukker døren 
     //sjekk at man står i ro
     //hvis stoppknapp: dør åpen så elnge man holder inne +3 sek 
     if (elevio_obstruction() == 0 && elevio_floorSensor() != -1) {
         elevio_doorOpenLamp(1);
         sleep(3); 
         elevio_doorOpenLamp(0);
     }
}

void stopButton(struct StateMachine *state) {
    // Når trykkes inn:
    // knapp lyser
    if (elevio_stopButton() == 1 && state->active == 1) { //stoppknapp trykkes inn
        state->active = 0;

        state->direction = DIRN_STOP;
        if (elevio_floorSensor() == -1) { //mellom etasjer
            printf("Between floors, can´t open door");
        }
        if (elevio_floorSensor() == 1) { //i en etasje
            elevio_doorOpenLamp(1);
        }
        emptyQueue(state);
    }
    else if (elevio_stopButton() == 1 && state->active == 0) {
        state->active = 1;
    }
    if (elevio_stopButton() == 1) {
        elevio_stopLamp(1);
    }
    else {
        elevio_stopLamp(0);
    }
    // heis stopper momentant
    // sjekk om i en etasje
    // hvis ja, hold dør åpen helt til slippes + 3 sek 
    // slett bestillingskø
    // ikke tillate flere bestillinger
    // stå i ro

}

void emptyQueue(struct StateMachine *state) {
    for (int i = 0; i < state->orderCount; i++){
        state->queue[i] = -1;
    }
    state->orderCount = 0;
    printf("Emptying queue\n");    
}

void floorIndicator(struct StateMachine *state) {
    elevio_floorIndicator(state->currentFloor);
}