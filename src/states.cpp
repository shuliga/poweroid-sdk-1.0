//
// Created by SHL on 28.04.2021.
//

#include "states.h"

template<class T>
void StateHolder<T>::gotoState(T newState) {
    prev_state = state;
    state = newState;
}

template<class T>
bool StateHolder<T>::isDisarmed() {
    return state == disarmed_state;
}

template<class T>
void StateHolder<T>::disarm(bool disarm) {
    gotoState(disarm ? disarmed_state : init_state);
}

template<class T>
T StateHolder<T>::getState() {
    return state;
}

