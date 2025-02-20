#pragma once
#include <stdlib.h>
#include "elevio.h"

#define MAX_ORDERS 15

struct StateMachine {
    int currentFloor;
    int nextFloor;
    int direction;
    int active;          // om heisen er i bevegelse
    int queue[MAX_ORDERS]; // lagrer etasjene med bestillinger
    int orderCount;      // antall bestillinger i køen
};

void getToFirstFloor();
void init(struct StateMachine *state);
void getOrders(struct StateMachine *state);
void nextFloor(struct StateMachine *state);
void openDoor();

void initQueue(struct StateMachine *state);
void addOrder(struct StateMachine *state, int floor);
void removeOrder(struct StateMachine *state, int floor);
int getNextOrder(struct StateMachine *state);
