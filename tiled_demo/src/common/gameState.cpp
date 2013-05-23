#include <cassert>

#include "hge.h"

#include "common/gameState.h"
#include "common/profile.h"

void GameState::OnEnter()
{
}

void GameState::OnLeave()
{
}

void GameState::SetName(const string &name)
{
    if (!GameStateManager::getSingleton().FindState(name))
    {
        this->name = name;
    }
}

GameStateManager::GameStateManager() : curState(0)
{
    HGE *hge = hgeCreate(HGE_VERSION);
    assert(hge);
    hge->System_SetState(HGE_FRAMEFUNC, FrameFunc);
    hge->System_SetState(HGE_RENDERFUNC, RenderFunc);
    hge->Release();
}

GameStateManager::~GameStateManager()
{
    // must call releaseSingleton before me
    assert(!curState);
}
void GameStateManager::releaseSingleton()
{
    if (curState)
    {
        curState->OnLeave();
        curState = 0;
    }
}

GameState *GameStateManager::FindState(const string &name) const
{
    for (vector<GameState *>::const_iterator s = states.begin(); s != states.end(); ++s)
    {
        if ((*s)->GetName() == name)
            return *s;
    }
    return 0;
}

void GameStateManager::RegisterState(GameState *state)
{
    assert(state);
    assert(!state->GetName().empty());
    assert(0 == FindState(state->GetName()));
    states.push_back(state);
}

void GameStateManager::RequestState(const string &name)
{
    assert(requestState.empty()); // already requested
    requestState = name;
}

bool GameStateManager::OnFrame()
{
#if USE_PROFILER
    Profile::getSingleton().flushFrame();
#endif
    if (!requestState.empty())
    {
        if (curState)
        {
            curState->OnLeave();
            curState = 0;
        }
        if (requestState == "exit")
            return true;
        curState = FindState(requestState);
        requestState.clear();
        if (curState)
            curState->OnEnter();
    }

    if (curState)
        curState->OnFrame();

    return false;
}

static bool FrameFunc()
{
    return GameStateManager::getSingleton().OnFrame();
}

static bool RenderFunc()
{
    GameState *gs = GameStateManager::getSingleton().curState;
    if (gs)
    {
        gs->OnRender();
    }
    return false;
}
