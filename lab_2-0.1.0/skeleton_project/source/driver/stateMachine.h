#pragma once
#include <stdlib.h>
#include "elevio.h"

#define MAX_ORDERS 15

/**
 *Core structure for managing elevator state and order queue
 */
struct StateMachine {
    int currentFloor;           //Current floor position of elevator
    int nextFloor;              //Target floor to visit next
    int direction;              //Current movement direction (1=up, -1=down, 0=idle)
    int queue[MAX_ORDERS];      //Array of floor numbers with pending orders
    int orderCount;             //Number of orders currently in queue
    int active;                 //Whether the elevator is in operation (1) or not (0)
    ButtonType queueDirection[MAX_ORDERS]; //Type of button press for each order (up/down/cab)
    int stoppedBetweenFloors;   //Indicates if elevator is between floors (1) or at a floor (0)
};

//Function declarations remain unchanged
void getToFirstFloor(struct StateMachine *state);
void init(struct StateMachine *state);
void getOrders(struct StateMachine *state);
void nextFloor(struct StateMachine *state);
void openDoor(struct StateMachine *state);

void initQueue(struct StateMachine *state);
void addOrder(struct StateMachine *state, int floor, ButtonType btn);
void removeOrder(struct StateMachine *state, int floor);
int getNextOrder(struct StateMachine *state);
void stopButton(struct StateMachine *state);
void emptyQueue(struct StateMachine *state);
void floorIndicator(struct StateMachine *state);
