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
#include "verlet/verlet.h"
#include "game/dataCurvedAni.h"
#include "game/tankContactInfo.h"

using std::list;

class hgeFont;
class hgeSprite;
class CannonBall;
class Bullet;
class Map;
class AiTankController;
class RadarMap;
class TargetIndicator;
class CooldownIcon;

class MainGameState : public GameState, public Singleton<MainGameState>
{
public:
    MainGameState() : hge(0), frameCount(0)
    {
    }
    virtual ~MainGameState();

    virtual void OnEnter();
    virtual void OnLeave();
    virtual void OnFrame();
    virtual void OnRender();

    CannonBall *createCannonBall();
    Bullet *createBullet();

    hgeFont *getFont()
    {
        return font;
    }
    void resetMyTank();
    void addAiTank(const Point2f &pos, AiTankController *controller = 0);
    void addRandomItem(const Point2f &pos);
    void findPath(list<Point2f> &result, const Point2f &fromPos, const Point2f &toPos) const;
protected:
    void ProcessTankControl(const hgeInputEvent &event);

    void UpdateTankAim( bool turn );
    HGE *hge;
    HTEXTURE texGui;
    hgeGUI *gui;
    hgeFont *font;
    cAni::iAnimResManager *animResManager;
    Map *map;
    hgeCurvedAniSystem *system;
    SingleController tankController;
    TankCollisionChecker tankCollisionChecker;
    TankContactInfo tankContactInfo;
    RadarMap *radar;
    TargetIndicator *targetIndicator;
    CooldownIcon *gunCooldown;
    CooldownIcon *cannonCooldown;
    RenderQueue renderQueue;
    list<AiTankController*> aitankers;

    // renderers instances
    DataTriangleRenderer dataTriangleRenderer;

    int frameCount;
};

#endif
