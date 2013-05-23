#include <hge.h>
#include "staticMapObj.h"

//#include "game/mainGameState.h"
#include "game/tankContactInfo.h"

Entity::iEntity *StaticMapObj::getEntityInterface(EntityInterfaceId id)
{
    switch(id)
    {
    case EII_CollisionEntity:
        return &collisionEntity;
    case EII_GraphicEntity:
        return &graphicEntity;
    }

    return Entity::getEntityInterface(id);
}

void StaticMapObj::CollisionEntity::onCollision(iCollisionEntity &o, const iContactInfo &ci)
{
    HGE *hge = hgeCreate(HGE_VERSION);
    float deltaTime = hge->Timer_GetDelta();
    hge->Release();
    
    TankContactInfo &tci = (TankContactInfo &)ci;
    Point2f contactPos = tci.getData()->GetContact(0);
    Point2f dir = o.getBody().GetPointVelocity(contactPos, deltaTime);
    Point2f impulse = dir * 0.001f;// * o.getBody().getMass();
    Point2f force = impulse / deltaTime;
    getBody().AddForce(force, contactPos);
}

void StaticMapObj::GraphicEntity::render(float gameTime, float deltaTime)
{
    deltaTime;
    int frameTime = int(gameTime * 60);
    assert(animShape);
    iRenderQueue::getSingleton().render(getEntity().pos, -getEntity().orientation, frameTime, animShape, getEntity().getRenderLayerId());

#ifdef _DEBUG
#if 0
    const Rectf &r = getEntity().collisionEntity.getBoundingBox();
    iRenderQueue::getSingleton().render(Point2f(r.left, r.top), Point2f(r.right, r.top), ARGB(128, 255, 255, 255));
    iRenderQueue::getSingleton().render(Point2f(r.right, r.top), Point2f(r.right, r.bottom), ARGB(128, 0, 0, 255));
    iRenderQueue::getSingleton().render(Point2f(r.right, r.bottom), Point2f(r.left, r.bottom), ARGB(128, 0, 0, 255));
    iRenderQueue::getSingleton().render(Point2f(r.left, r.bottom), Point2f(r.left, r.top), ARGB(128, 0, 0, 255));
#endif
#endif
}