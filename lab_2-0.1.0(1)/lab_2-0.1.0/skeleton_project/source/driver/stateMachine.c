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
    elevio_doorOpenLamp(1);
    sleep(3); 
    elevio_doorOpenLamp(0);
}

void init(struct StateMachine *state){
    getToFirstFloor();
    state->currentFloor = elevio_floorSensor();
    state->direction = DIRN_STOP;
    printf("Elevator is in first floor\n");
}

void initQueue(struct StateMachine *state) {
    for (int i = 0; i < MAX_ORDERS; i++){
        state->queue[i] = -1;
    }
    state->orderCount = 0;
}

void addOrder(struct StateMachine *state, int floor) {
    for (int i = 0; i < state->orderCount; i++){
        if (state->queue[i] == floor)
            return;
    }
    if (state->orderCount < MAX_ORDERS){
        state->queue[state->orderCount] = floor;
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
        state->queue[state->orderCount] = -1;
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
                addOrder(state, floor);
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

    if (state->currentFloor < next) {
        state->direction = DIRN_UP;
    } else if (state->currentFloor > next) {
        state->direction = DIRN_DOWN;
    } else {
        state->direction = DIRN_STOP;
        removeOrder(state, next);
        elevio_motorDirection(state->direction);
        return;
    }
    elevio_motorDirection(state->direction);
    printf("Heisen beveger seg %s mot etasje %d. Current floor: %d\n", 
        (state->direction == DIRN_UP) ? "opp" : "ned", next + 1, current + 1);

}
