#ifndef MAINGAMESTATE_H
#define MAINGAMESTATE_H

#include <list>

#include "hgegui.h"
#include "hgecolor.h"
#include "curvedani.h"
#include "hgeCurvedAni.h"
#include "common/gameState.h"
#include "common/entity.h"
#include "common/controlEntity.h"
#include "common/utils.h"
#include "common/entityManager.h"
#include "common/graphicEntity.h"
#include "common/renderQueue.h"
#include "game/dataCurvedAni.h"

using std::list;

class hgeFont;
class hgeSprite;
class TiledMapEntity;
class MainGameState : public GameState, public Singleton<MainGameState>
{
public:
    MainGameState() : hge(0)
    {
    }
    virtual ~MainGameState();

    virtual void OnEnter();
    virtual void OnLeave();
    virtual void OnFrame();
    virtual void OnRender();

    hgeFont *getFont()
    {
        return font;
    }
protected:
    void ProcessControl(const hgeInputEvent &event);

    HGE *hge;
    HTEXTURE texGui;
    hgeGUI *gui;
    hgeFont *font;
    cAni::iAnimResManager *animResManager;
    TiledMapEntity *map;
    hgeCurvedAniSystem *system;
    SingleController tankController;
    RenderQueue renderQueue;

    // renderers instances
    DataTriangleRenderer dataTriangleRenderer;
};

#endif
