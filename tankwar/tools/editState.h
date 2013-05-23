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
#include "verlet/verlet.h"
#include "game/dataCurvedAni.h"
#include "common/renderQueue.h"

using std::list;

class hgeFont;
class hgeSprite;
class Map;
class AiTankController;
class TankContactInfo : public iContactInfo
{
public:
    verlet::Contact *getData()
    {
        return contact;
    }
    const verlet::Contact *getData() const
    {
        return contact;
    }
    void setNumber(size_t num)
    {
        this->num = num;
    }
    size_t getNumber() const
    {
        return num;
    }
    virtual void Update()
    {
        static Point2f off[4] =
        {
            Point2f(-3, -3),
            Point2f(3, -3),
            Point2f(3, 3),
            Point2f(-3, 3),
        };
        for(size_t i = 0; i < num; i++)
        {
            //RenderQueue::getSingleton().render(contact[i].m_xContacts[0] + off[0], contact[i].m_xContacts[1] + off[2], ARGB(255, 0, 255, 0));
            //RenderQueue::getSingleton().render(contact[i].m_xContacts[0] + off[1], contact[i].m_xContacts[1] + off[3], ARGB(255, 0, 255, 0));
            contact[i].ResolveOverlap();
            contact[i].ResolveCollision();
        }
    }
protected:
    verlet::Contact contact[8];
    size_t num;

};
class TankCollisionChecker : public CollisionChecker
{
public:
    bool checkCollision(iCollisionEntity *a, iCollisionEntity *b, iContactInfo &contactInfo);

    int checkCount;
//protected:
    float deltaTime;
};

class EditState : public GameState, public Singleton<EditState>
{
public:
    EditState() : hge(0), bLeft(false), bRight(false), bUp(false), bDown(false),
        bMouseLeft(false), bMouseRight(false), bMouseUp(false), bMouseDown(false)
    {
        pathFindingOp = 0;
    }
    virtual ~EditState();

    virtual void OnEnter();
    virtual void OnLeave();
    virtual void OnFrame();
    virtual void OnRender();

    hgeFont *getFont()
    {
        return font;
    }
    void addAiTank(const Point2f &pos, AiTankController *controller = 0);
    void findPath(list<Point2f> &result, const Point2f &fromPos, const Point2f &toPos) const;
protected:
    HGE *hge;
    HTEXTURE texGui;
    hgeGUI *gui;
    hgeFont *font;
    cAni::iAnimResManager *animResManager;
    Map *map;
    hgeCurvedAniSystem *system;
    TankCollisionChecker tankCollisionChecker;
    TankContactInfo tankContactInfo;
    RenderQueue renderQueue;
    list<AiTankController*> aitankers;

    bool bLeft, bRight, bUp, bDown;
    bool bMouseLeft, bMouseRight, bMouseUp, bMouseDown;

    int pathFindingOp;
    Point2f pathFindingFrom, pathFindingTo;
    list<Point2f> pathFindingResult;

    AiTankController *aiController;
};

#endif
