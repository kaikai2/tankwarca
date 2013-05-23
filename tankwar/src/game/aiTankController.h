#ifndef AI_TANK_CONTROLLER_H
#define AI_TANK_CONTROLLER_H

#include <queue>
#include <list>

#include "caPoint2d.h"
#include "common/controlEntity.h"

using std::queue;
using std::list;
using cAni::Point2f;

class Map;

class AiTankController : public SingleController
{
public:
    AiTankController();
    virtual ~AiTankController();

    virtual bool think(float gameTime, float deltaTime, bool bAvoidTimeConsumingMethod);
    void attacheMap(Map *map);
    
    void goTo(const Point2f &pos); // findPath and go
    void resetOperates();

protected:
    void OnDettach(iAttacheeEntity &entity);

    void moveTo(const Point2f &pos, const Point2f &nextPos); // directly move
    void turnTo(const Point2f &pos);
    void aimTo(const Point2f &pos);

    bool findPathTo(const Point2f &pos, list<Point2f> &result);

    void stepMoveTo();

    // this may be time consuming method
    bool scanAround(float gameTime); 

    Entity &getTankEntity()
    {
        return entity->getEntity();
    }
    bool bForward;
    bool bLeft;
    bool bRight;
    bool bBackward;
    bool bFire;
    bool bShoot;

    bool bCheckMoveTo;
    Point2f moveToPos, nextMoveToPos, animPos;
    float lastScanTime;

    rEntity target;
    struct Operate
    {
        enum CommandId
        {
            CI_MoveTo,
        };
        Operate(CommandId id) : commond(id)
        {
        }
        CommandId commond;
        list<Point2f> wayPoints;
    };
    queue<Operate> operates;

    SingleAttacher mapAttacher;

    float fScanAroundSkipTime;

};

#endif//AI_TANK_CONTROLLER_H