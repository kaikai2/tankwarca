#include "common/collisionSpace.h"
#include "common/entityManager.h"
#include "common/profile.h"

#include "gameDef.h"
#include "aiTankController.h"
#include "tank.h"
#include "map.h"

class AiSightEntityFilter : public EntityFilter
{
public:
    AiSightEntityFilter(const Point2f &_pos, const Entity &_entity) : pos(_pos), entity(_entity)
    {
        boundingBox.leftTop = pos + Point2f(-300, -200);
        boundingBox.rightBottom = pos + Point2f(300, 200);
    }
    const Rectf &getBoundingBox() const
    {
        return boundingBox;
    }
    bool filtrate(Entity &e) const
    {
        switch(e.getTypeId())
        {
        case ETI_Tank:
            if (&e == &entity)
                break;

            // no break here
        case ETI_Item:
            {
                Point2f ePos;
                if (e.getStatus(Entity::ESI_Position, &ePos) && (ePos - pos).DotProduct() < 300 * 300)
                {
                    return true;
                }
            }
            break;
        }
        return false;
    }
protected:
    Rectf boundingBox;
    Point2f pos;
    const Entity &entity;
};

AiTankController::AiTankController()
{
    bForward = false;
    bLeft = false;
    bRight = false;
    bBackward = false;
    bFire = false;
    bShoot = false;

    bCheckMoveTo = false;
    lastScanTime = 0;

    fScanAroundSkipTime = rand() % 10 * 0.2f + 5.f;
}

AiTankController::~AiTankController()
{

}

void AiTankController::attacheMap(Map *map)
{
    ((iAttacheeEntity*)map->getEntityInterface(Map::EII_QueryEntity))->attach(mapAttacher);
}

bool AiTankController::scanAround(float gameTime)
{
    if (!isAttached())
        return false;

    if (gameTime < lastScanTime + fScanAroundSkipTime)
        return false;

    lastScanTime = gameTime;

    vector<Entity *> entities;
    Point2f posTank;
    Entity &tankEntity = getTankEntity();
    if (!tankEntity.getStatus(Entity::ESI_Position, &posTank))
        return false;
    PROFILE_BEGINPOINT(AiTankController::scanAroundA);
    EntityManager::getSingleton().getEntities(entities, AiSightEntityFilter(posTank, tankEntity));
    PROFILE_CHECKPOINT(AiTankController::scanAroundB);
    Point2f aimPos;
    float aimDistance = 1001.f;
    for (size_t i = 0; i < entities.size(); i++)
    {
        Entity *e = entities[i];
        Point2f ePos;
        switch(e->getTypeId())
        {
        case ETI_Tank:
            if (e != &tankEntity && e->getStatus(Entity::ESI_Position, &ePos))
            {
                float distance = (ePos - posTank).Length();
                if (distance > 50.f && distance < aimDistance)
                {
                    aimDistance = distance;
                    aimPos = ePos;
                    target = e;
                }
            }
            break;
        case ETI_Item:
            if (operates.empty() && e->getStatus(Entity::ESI_Position, &ePos))
            {
                Operate moveTo(Operate::CI_MoveTo);
                if (findPathTo(ePos, moveTo.wayPoints))
                {
                    operates.push(moveTo);
                    i = entities.size();
                    break;
                }
                else
                {
                    // just give up the item
                    // moveTo(ePos);
                }
            }
            break;
        }
    }
    PROFILE_ENDPOINT();
    return true;
}
bool AiTankController::think(float gameTime, float deltaTime, bool bAvoidTimeConsumingMethod)
{
    deltaTime;

    if (!isAttached())
        return false;
    if (target.getObj())
    {
        Point2f posTank;
        Entity &e = entity->getEntity();
        if (e.getStatus(Entity::ESI_Position, &posTank))
            iRenderQueue::getSingleton().render(animPos, posTank, ARGB(255, 0, 0, 255));
    }
#ifdef PATH_FINDING_TOOL
//#ifndef NDEBUG
    

#endif

    if (!operates.empty())
    {
        Operate &op = operates.back();
        switch(op.commond)
        {
        case Operate::CI_MoveTo:
            if (!op.wayPoints.empty())
            {
                Point2f lastPos = moveToPos;
                for (list<Point2f>::iterator ip = op.wayPoints.begin(); ip != op.wayPoints.end(); ++ip)
                {
                    iRenderQueue::getSingleton().render(*ip, lastPos, ARGB(255, 128, 0, 0));
                    lastPos = *ip;
                }
            }
            break;
        }
    }
    // check for commands list
    if (bCheckMoveTo)
    {
        stepMoveTo();
    }
    else if (!operates.empty())
    {
        Operate &op = operates.front();
        switch(op.commond)
        {
        case Operate::CI_MoveTo:
            if (!op.wayPoints.empty())
            {
                Point2f pos = op.wayPoints.front();
                op.wayPoints.pop_front();
                if (op.wayPoints.empty())
                    moveTo(pos, pos);
                else
                    moveTo(pos, op.wayPoints.front());
            }
            else
            {
                operates.pop();
            }
            break;
        }
    }

    int r = rand() % 1000;
    // query environments (for updating covers and enemies)
    // determine the currently most important task(to track enemies or to escape from the enemies or go to the nearest supply zone, etc.)
    // generate path
    bool bRet = false;
    if (r > 500 && !bAvoidTimeConsumingMethod)
    {
        bRet = scanAround(gameTime);
    }

    // attack or rest
    if (target.getObj())
    {
        if (r < 5)
        {
            sendCommand(Tank::TCI_Fire, (void *)(bFire = !bFire));
        }
        else if (r < 20)
        {
            sendCommand(Tank::TCI_Shoot, (void *)(bShoot = !bShoot));
        }
    }
    else
    {
        // no target, seize fire
        if (bFire)
            sendCommand(Tank::TCI_Fire, (void *)(bFire = !bFire));
        if (bShoot)
            sendCommand(Tank::TCI_Shoot, (void *)(bShoot = !bShoot));
    }
    Entity *targetObj = (Entity *)target.getObj();
    if (targetObj)
    {
        if (targetObj->isActive())
        {
            Point2f posTank;
            Entity &e = entity->getEntity();
            if (e.getStatus(Entity::ESI_Position, &posTank))
            {
                Point2f ePos, eVel;
                if (targetObj->getStatus(Entity::ESI_Position, &ePos) &&
                    targetObj->getStatus(Entity::ESI_Velocity, &eVel))
                {
                    float distance = (ePos - posTank).Length();
                    if (distance < 300.f)
                    {
                        const float averWeaponSpeed = 300.0f;
                        float time = distance / averWeaponSpeed;
                        aimTo(ePos + eVel * time);
                    }
                    else
                    {
                        target.release();
                    }
                }
            }
        }
        else
        {
            target.release();
        }
    } 
    return bRet;
}

void AiTankController::goTo(const Point2f &pos) // findPath and go
{
    if (!isAttached())
        return;

    Operate moveTo(Operate::CI_MoveTo);
    if (findPathTo(pos, moveTo.wayPoints))
    {
        operates.push(moveTo);
    }
}

void AiTankController::resetOperates()
{
    bCheckMoveTo = false;
    while(!operates.empty())
        operates.pop();
}

void AiTankController::moveTo(const Point2f &pos, const Point2f &nextPos)
{
    if (!isAttached())
        return;

    turnTo(pos);
    bCheckMoveTo = true;
    moveToPos = pos;
    nextMoveToPos = nextPos;
}

void AiTankController::turnTo(const Point2f &pos)
{
    sendCommand(Tank::TCI_Turn, &pos);
}

void AiTankController::aimTo(const Point2f &pos)
{
    animPos = pos;
    sendCommand(Tank::TCI_Aim, &pos);
}

void AiTankController::stepMoveTo()
{
    if (!isAttached())
    {
        bCheckMoveTo = false;
        return;
    }
    Point2f posTank;
    float orientionTank;
    Entity &e = entity->getEntity();
    if (e.getStatus(Entity::ESI_Position, &posTank) &&
        e.getStatus(Entity::ESI_Orientation, &orientionTank))
    {
    //#ifndef NDEBUG
        iRenderQueue::getSingleton().render(moveToPos, posTank, ARGB(255, 0, 0, 0));
//#endif
        Point2f dir = moveToPos - posTank;
        float len = dir.Length();
        float len2 = (nextMoveToPos - posTank).Length();
        if (len < 10 || len > len2)
        {
            bCheckMoveTo = false;
            bForward = false;
            sendCommand(Tank::TCI_Forward, (void *)bForward);
            return;
        }
        dir.Normalize();
        if (fabs(dir ^ Point2f(cos(orientionTank), sin(orientionTank))) < 0.3f)
        {
            if (!bForward)
            {
                bForward = true;
                sendCommand(Tank::TCI_Forward, (void *)bForward);
            }
        }
        else
        {
            float lerpValue = (float)rand() / RAND_MAX;
            lerpValue *= lerpValue;
            turnTo(moveToPos);//lerp(moveToPos, nextMoveToPos, lerpValue));
            if (bForward)
            {
                bForward = false;
                sendCommand(Tank::TCI_Forward, (void *)bForward);
            }
        }
    }
    else
    {
        bCheckMoveTo = false;
    }
}

bool AiTankController::findPathTo(const Point2f &pos, list<Point2f> &result)
{
    // get map info,
    // generate rough path to pos
    // bla bla bla

    Point2f posTank;
    Entity &tankEntity = getTankEntity();
    if (tankEntity.getStatus(Entity::ESI_Position, &posTank))
    {
        if (mapAttacher.isAttached())
        {
            const Map *map = (const Map *)mapAttacher.getEntity();
            if (map)
            {
                map->findPath(result, posTank, pos);
                return true;
            }
        }
    }
    return false;
}

void AiTankController::OnDettach(iAttacheeEntity &entity)
{
    entity;

    this->target.release();

    while(!operates.empty())
        operates.pop();
    bCheckMoveTo = false;
}
