#pragma once
#include <stdlib.h>

struct StateMachine {
    int currentFloor;
    int nextFloor;
    int direction;
    int active; //om den kjører eller ikke
    int queue[15]; //kan ikke inisialisere en dynamisk array i struct
};

void getToFirstFloor();
void init();
void getOrders(struct StateMachine *state);
