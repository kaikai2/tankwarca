#ifndef COMMON_STATE_MACHINE_H
#define COMMON_STATE_MACHINE_H

class StateMachine
{
public:
    typedef unsigned long State;
    enum StateIds
    {
        SI_Enter,    // ³õÊ¼×´Ì¬
        SI_Exit,     // ½áÊø

        NumStateIds,
    };
    StateMachine() : currentState(SI_Enter)
    {

    }
    virtual void step();
    State getCurrentState() const
    {
        return currentState;
    }
protected:
    State currentState;
};

#endif//COMMON_STATE_MACHINE_H