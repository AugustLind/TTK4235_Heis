#include "stateMachine.h"
#include "elevio.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>

/* Constants to avoid magic numbers */
#define DOOR_OPEN_TIME_SEC 3
#define GROUND_FLOOR 0
#define INVALID_FLOOR -1
#define BUTTON_HALL_DOWN_VALUE 1
#define BUTTON_HALL_UP_VALUE 0

/* 
 * Moves elevator to the ground floor (floor 0) on initialization
 * and sets up initial state
 */
void getToFirstFloor(struct StateMachine *state){
    elevio_motorDirection(DIRN_DOWN);
    while(elevio_floorSensor() != GROUND_FLOOR) {
        // Continue going down until we hit ground floor
    }
    elevio_motorDirection(DIRN_STOP);
    for (ButtonType btn = BUTTON_HALL_UP; btn <= BUTTON_CAB; btn++){
        for (int floor = 0; floor < N_FLOORS; floor++){
            elevio_buttonLamp(floor, btn, 0);
        }
    }
    elevio_floorIndicator(GROUND_FLOOR);
    openDoor(state);
}

/* 
 * Initializes the elevator state machine with default values
 * and positions the elevator at the first floor
 */
void init(struct StateMachine *state){
    state->currentFloor = INVALID_FLOOR;
    state->nextFloor = INVALID_FLOOR;
    state->direction = DIRN_STOP;
    state->orderCount = 0;
    state->active = 1;
    state->stoppedBetweenFloors = 0;
    
    getToFirstFloor(state);
    state->currentFloor = elevio_floorSensor();
    printf("Elevator is in first floor\n");
}

/* 
 * Initializes the elevator order queue to empty state
 */
void initQueue(struct StateMachine *state) {
    for (int i = 0; i < MAX_ORDERS; i++){
        state->queue[i] = INVALID_FLOOR;
        state->queueDirection[i] = BUTTON_CAB;
    }
    state->orderCount = 0;
}
 
/* 
 * Adds a new floor order to the queue if not already present
 */
void addOrder(struct StateMachine *state, int floor, ButtonType btn) {
    for (int i = 0; i < state->orderCount; i++){
        if (state->queue[i] == floor)
            return;
    }
    if (state->orderCount < MAX_ORDERS){
        state->queue[state->orderCount] = floor;
        state->queueDirection[state->orderCount] = btn;
        state->orderCount++;
        //printf("La til bestilling for etasje %d\n", floor + 1);
        //printf("Med rettning %d\n", btn);
    } else {
        //printf("Køen er full\n");
    }
}

/* 
 * Removes a floor order from the queue and turns off its button lamps
 */
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
            state->queueDirection[i] = state->queueDirection[i+1];
        }
        elevio_buttonLamp(floor, BUTTON_HALL_UP, 0);
        elevio_buttonLamp(floor, BUTTON_HALL_DOWN, 0);
        elevio_buttonLamp(floor, BUTTON_CAB, 0);
        state->queue[state->orderCount] = INVALID_FLOOR;
        state->queueDirection[state->orderCount] = BUTTON_CAB;
        state->orderCount--;
        //printf("Fjernet bestilling for etasje %d\n", floor + 1);
    }
}

/* 
 * Determines the next floor to visit based on current direction and orders
 * Uses elevator direction to prioritize orders:
 * - When moving up: prioritizes floors above current position (excluding down requests)
 * - When moving down: prioritizes floors below current position (excluding up requests)
 * - Returns closest floor in the current direction of travel
 */
int getNextOrder(struct StateMachine *state) {
    if (state->orderCount == 0)
        return state->currentFloor;
    int nextFloor = state->queue[0];
    for (int i = 0; i < state->orderCount; i++) {
        int floor = state->queue[i];
        ButtonType btn = state->queueDirection[i];
        //printf("Etasje: %d\n", floor + 1);
        //printf("Knapp:  %d\n", btn);
        
        // When moving up, look for orders above current floor, excluding down requests (btn=BUTTON_HALL_DOWN_VALUE)
        if (state->direction == DIRN_UP && floor > state->currentFloor && btn != BUTTON_HALL_DOWN_VALUE) {
            // Choose this floor if it's closer than current target or if current target is below us
            if (floor < nextFloor || nextFloor < state->currentFloor) {
                nextFloor = floor;
                //printf("Byttet etasje med retning %d\n", btn);
            } 
        } 
        // When moving down, look for orders below current floor, excluding up requests (btn=BUTTON_HALL_UP_VALUE)
        else if (state->direction == DIRN_DOWN && floor < state->currentFloor && btn != BUTTON_HALL_UP_VALUE) {
            // Choose this floor if it's closer than current target or if current target is above us
            if (floor > nextFloor || nextFloor > state->currentFloor) {
                nextFloor = floor;
                //printf("Byttet etasje med retning %d\n", btn);
            }
        }
    }
    return nextFloor;
}

/* 
 * Scans for button presses and adds new orders to the queue
 */
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

/* 
 * Manages elevator movement to the next requested floor
 * Handles different scenarios:
 * 1. Normal operation - moves elevator toward next order
 * 2. Stopped between floors - special handling to resume correctly
 * 3. Arrival at destination - stops, removes order, opens door
 */
void nextFloor(struct StateMachine *state) {
    if (state->orderCount == 0) {
        return;
    }
    int next = getNextOrder(state);
    int current = elevio_floorSensor();
    if (current != INVALID_FLOOR){
        state->currentFloor = current;
    }
    floorIndicator(state);
    
    // Special handling for elevator stopped between floors
    if (state->stoppedBetweenFloors == 1 && state->active == 1) {
        //printf("Starter mellom etasjer: %d\n", state->direction);
        //printf("%d\n", next);
        
        // If we've reached the target floor
        if (elevio_floorSensor() == next){ 
            state->stoppedBetweenFloors = 0;
            state->direction = DIRN_STOP;
            elevio_motorDirection(state->direction);
        } 
        // If we were moving down when stopped
        else if (state->direction == DIRN_DOWN) {
            // Continue in appropriate direction based on next floor position
            if (next <= state->currentFloor - 1) {
                elevio_motorDirection(DIRN_DOWN); 
            } else {
                elevio_motorDirection(DIRN_UP);   
            }
        }
        // If we were moving up when stopped
        else if (state->direction == DIRN_UP) {
            // Continue in appropriate direction based on next floor position
            if (next >= state->currentFloor + 1) {
                elevio_motorDirection(DIRN_UP); 
            } else {
                elevio_motorDirection(DIRN_DOWN); 
            }
        } 
    }
    // Normal elevator movement logic
    else {           
        if (state->currentFloor < next && elevio_floorSensor() != next) {
            state->direction = DIRN_UP;
        } else if (state->currentFloor > next && elevio_floorSensor() != next) {
            state->direction = DIRN_DOWN;
        } else {
            // We've arrived at the destination floor
            state->direction = DIRN_STOP;
            elevio_motorDirection(state->direction);
            int count = state->orderCount;
            removeOrder(state, next);
            if (count > 0) {
                openDoor(state);
            }
           
            return;
        }
        elevio_motorDirection(state->direction);
    }
    //printf("Heisen beveger seg %s mot etasje %d. Current floor: %d\n", 
        //(state->direction == DIRN_UP) ? "opp" : "ned", next + 1, current + 1);
}

/* 
 * Opens the elevator door when at a floor and keeps it open for 3 seconds
 * Handles obstruction sensor to keep door open longer if needed
 */
void openDoor(struct StateMachine *state) {
    if (elevio_floorSensor() != INVALID_FLOOR) {
        elevio_doorOpenLamp(1);
        while (elevio_obstruction())
        {} 
        time_t startTime = time(NULL);
        time_t end_time = startTime + DOOR_OPEN_TIME_SEC;
        while (time(NULL) < end_time) {
            getOrders(state);
        }
        elevio_doorOpenLamp(0);
    }
}

/* 
 * Handles stop button presses, stopping elevator operation
 * and clearing all orders when pressed
 */
void stopButton(struct StateMachine *state) {
    if (elevio_stopButton() == 1) {
        elevio_stopLamp(1);
        if (state->active == 1) {
            state->active = 0;
            elevio_motorDirection(DIRN_STOP);
            emptyQueue(state);

            if (elevio_floorSensor() != INVALID_FLOOR) {
                elevio_doorOpenLamp(1);
            } else {
                state->stoppedBetweenFloors = 1;
                //printf("Elevator between floors; door remains closed.\n");
            }
        }
    } else {
        elevio_stopLamp(0);
        if (state->active == 0 && elevio_floorSensor() != INVALID_FLOOR) {
            sleep(DOOR_OPEN_TIME_SEC);
            elevio_doorOpenLamp(0);
        }
        state->active = 1;
    }
}

/* 
 * Clears all orders from the queue and turns off all button lamps
 */
void emptyQueue(struct StateMachine *state) {
    for (int i = 0; i < state->orderCount; i++){
        state->queue[i] = INVALID_FLOOR;
    }
    state->orderCount = 0;
    for (ButtonType btn = BUTTON_HALL_UP; btn <= BUTTON_CAB; btn++){
        for (int floor = 0; floor < N_FLOORS; floor++){
            elevio_buttonLamp(floor, btn, 0);
        }
    }
    //printf("Emptying queue\n");    
}

/* 
 * Updates the floor indicator display based on current position
 */
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