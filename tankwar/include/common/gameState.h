#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <string>
#include <vector>
#include "common/utils.h"

using std::string;
using std::vector;

class GameState : public NoCopy
{
public:
    GameState()
    {
    }
    virtual void OnEnter();
    virtual void OnLeave();

    virtual void OnFrame() {}
    virtual void OnRender() {}

    void SetName(const string &name);
    const string &GetName() const
    {
        return name;
    }

protected:
    void RequestState(const string &name) const;

    string name;
};

class GameStateManager : public Singleton<GameStateManager>
{
public:
    GameStateManager();
    virtual ~GameStateManager();
    virtual void releaseSingleton();

    void RegisterState(GameState *state);
    void RequestState(const string &name);
    GameState *FindState(const string &name) const;

protected:
    bool OnFrame();

    friend static bool FrameFunc();
    friend static bool RenderFunc();

    vector<GameState *> states;
    string requestState;
    GameState *curState;
};

inline
void
GameState::RequestState(const string &name) const
{
    GameStateManager::getSingleton().RequestState(name);
}

#endif