#include "stateMachine.h"
#include "elevio.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // for sleep()
#include <time.h>

void getToFirstFloor(struct StateMachine *state){
    elevio_motorDirection(DIRN_DOWN);
    while(elevio_floorSensor() != 0) {
    
    }
    elevio_motorDirection(DIRN_STOP);
    for (ButtonType btn = BUTTON_HALL_UP; btn <= BUTTON_CAB; btn++){
        for (int floor = 0; floor < N_FLOORS; floor++){
            elevio_buttonLamp(floor, btn, 0);
        }
    }
    elevio_floorIndicator(0);
    openDoor(state);
}

void init(struct StateMachine *state){
    // Initialize struct members to avoid issues with uninitialized memory
    state->currentFloor = -1; // Or an appropriate value
    state->nextFloor = -1;
    state->direction = DIRN_STOP;
    state->orderCount = 0;
    state->active = 1;
    state->stoppedBetweenFloors = 0;
    
    // Call function to initialize other components
    getToFirstFloor(state);
    state->currentFloor = elevio_floorSensor();
    printf("Elevator is in first floor\n");
}

void initQueue(struct StateMachine *state) {
    for (int i = 0; i < MAX_ORDERS; i++){
        state->queue[i] = -1;
        state->queueDirection[i] = BUTTON_CAB;
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
        state->queueDirection[state->orderCount] = btn;
        state->orderCount++;
        printf("La til bestilling for etasje %d\n", floor + 1);
        printf("Med rettning %d\n", btn);
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
        state->queueDirection[state->orderCount] = BUTTON_CAB;
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
        ButtonType btn = state->queueDirection[i];
        
        if (state->direction == DIRN_UP && floor > state->currentFloor && btn != 1) {
            if (floor < nextFloor || nextFloor < state->currentFloor) {
                nextFloor = floor;
                printf("Byttet etasje med retning %d\n", btn);
            } 
        } else if (state->direction == DIRN_DOWN && floor < state->currentFloor && btn != 0) {
            if (floor > nextFloor || nextFloor > state->currentFloor) {
                nextFloor = floor;
                printf("Byttet etasje med retning %d\n", btn);
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
    if (state->orderCount == 0) {
        return;
    }
    int next = getNextOrder(state);
    int current = elevio_floorSensor();
    if (current != -1){
        state->currentFloor = current;
    }
    floorIndicator(state);
    if (state->stoppedBetweenFloors == 1 && state->active == 1) {
        printf("Starter mellom etasjer: %d\n", state->direction);
        printf("%d\n", next);
         if (elevio_floorSensor() == next){ 
             state->stoppedBetweenFloors = 0;
             state->direction = DIRN_STOP;
             elevio_motorDirection(state->direction);
         } 
         else if (state->direction == DIRN_DOWN) {
             if (next <= state->currentFloor - 1) {
                 elevio_motorDirection(DIRN_DOWN); 
             } else {
                 elevio_motorDirection(DIRN_UP);   
             }
         }
         else if (state->direction == DIRN_UP) {
             if (next >= state->currentFloor + 1) {
                 elevio_motorDirection(DIRN_UP); 
             } else {
                 elevio_motorDirection(DIRN_DOWN); 
             }
         } 
     }
    else{           
        if (state->currentFloor < next && elevio_floorSensor() != next) {
            state->direction = DIRN_UP;
        } else if (state->currentFloor > next && elevio_floorSensor() != next) {
            state->direction = DIRN_DOWN;
        } else {
            state->direction = DIRN_STOP;
            elevio_motorDirection(state->direction);
            if (state->orderCount > 0) {
                openDoor(state);
            }
            removeOrder(state, next);
            return;
        }
        elevio_motorDirection(state->direction);
        }
    printf("Heisen beveger seg %s mot etasje %d. Current floor: %d\n", 
        (state->direction == DIRN_UP) ? "opp" : "ned", next + 1, current + 1);

}

void openDoor(struct StateMachine *state) {
     //åpner og lukker døren 
     //sjekk at man står i ro
     //hvis stoppknapp: dør åpen så elnge man holder inne +3 sek 
     if (elevio_floorSensor() != -1) {
         elevio_doorOpenLamp(1);
         while (elevio_obstruction())
         {} 
        time_t startTime = time(NULL);
        time_t end_time = startTime + 3;
        while (time(NULL) < end_time) {
            getOrders(state);
        }
        elevio_doorOpenLamp(0);
     }
}
    

void stopButton(struct StateMachine *state) {
    if (elevio_stopButton() == 1) {
        // Stop button is pressed: light it and, if not already stopped, take action.
        elevio_stopLamp(1);
        if (state->active == 1) {
            state->active = 0;               // ignore new orders while stopped
            elevio_motorDirection(DIRN_STOP);
            emptyQueue(state);               // clear all orders
            

            if (elevio_floorSensor() != -1) {
                // If at a floor: open the door as required by D3.
                elevio_doorOpenLamp(1);
            } else {
                // Between floors: no door opening per requirements.
                state->stoppedBetweenFloors = 1;
                printf("Elevator between floors; door remains closed.\n");
            }
        }
        // While the button remains pressed, the elevator remains stopped.
    } else {
        // Button has been released.
        elevio_stopLamp(0);
        if (state->active == 0 && elevio_floorSensor() != -1) {
            // If the elevator stopped at a floor, keep the door open for an additional 3 sec.
            sleep(3);
            elevio_doorOpenLamp(0);
        }
        // Reactivate the elevator to allow new orders.
        state->active = 1;
    }
}

void emptyQueue(struct StateMachine *state) {
    for (int i = 0; i < state->orderCount; i++){
        state->queue[i] = -1;
    }
    state->orderCount = 0;
    for (ButtonType btn = BUTTON_HALL_UP; btn <= BUTTON_CAB; btn++){
        for (int floor = 0; floor < N_FLOORS; floor++){
            elevio_buttonLamp(floor, btn, 0);
        }
    } //clearing lights 
    printf("Emptying queue\n");    
}

void floorIndicator(struct StateMachine *state) {
    int next = getNextOrder(state);
    int current = elevio_floorSensor();
    if (next == current) {
        elevio_floorIndicator(next);
    }
    else {
        elevio_floorIndicator(state->currentFloor);
    }
}