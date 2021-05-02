//
// Created by SHL on 28.04.2021.
//

#ifndef STATES_H
#define STATES_H

typedef struct RunState {
    RunState() = default;
    char *name;
    char *state;
    StateHolder *stateHolder;
};

class StateHolderBase {
public:
    StateHolderBase( const char *stateName, const char *states[]) : stateName(stateName), states(states) {};
protected:
    const char *stateName;
    const char *states[];
}

template <class T>
class StateHolder : StateHolderBase {
public:
    StateHolder(T prev_state, T state, T disarmed_state, const char *stateName, const char *states[] ) : prev_state(prev_state), state(state), disarmed_state(disarmed_state),  StateHolderBase(stateName, states) {
        init_state = state;
    };
    void gotoState(T newState);
    bool isDisarmed();
    void disarm(bool disarm);
    T getState();

protected:
    T prev_state;
    T init_state;
    T state;
    T disarmed_state;
};

#endif //STATES_H
