//
// Created by SHL on 28.04.2021.
//

#ifndef STATES_H
#define STATES_H

#include <inttypes.h>

class StateHolderBase {
public:
    StateHolderBase( const char *name, const char *states[]) : name(name), states(states) {};
    virtual bool isDisarmed() = 0;
    virtual void disarm(bool disarm) = 0;
    virtual const char* getState() = 0;
    virtual bool wasChanged() = 0;
    bool changed = false;
    const char *name;
    const char **states;
};

template <class T>
class StateHolder : StateHolderBase {
public:
    StateHolder(T prev_state, T state, T disarmed_state, const char *stateName, const char *states[] ) : prev_state(prev_state), state(state), disarmed_state(disarmed_state),  StateHolderBase(stateName, states) {
        init_state = state;
    };
    bool isDisarmed();
    void disarm(bool disarm);
    const char* getState();
    bool wasChanged();
    void gotoState(T newState);
    T* firstEntry();
    T state;
    T prev_state;
protected:
    T init_state;
    T disarmed_state;
};

extern const uint8_t state_count;
extern StateHolderBase* run_states[];

template<class T>
void StateHolder<T>::gotoState(T newState) {
    prev_state = state;
    state = newState;
}

template<class T>
T* StateHolder<T>::firstEntry() {
    T* result = 0;
    if (prev_state != state) {
        result = &prev_state;
        prev_state = state;
        changed = true;
    } else {
        changed = false;
    }
    return result;
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
const char* StateHolder<T>::getState(){
    return states[state];
}

template<class T>
bool StateHolder<T>::wasChanged() {
    return changed;
}

#endif //STATES_H
