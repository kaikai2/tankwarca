#include <hge.h>
#include "common/entityManager.h"

#include "cannonBall.h"
#include "weapon.h"
#include "explosive.h"
//#include "game/mainGameState.h"
#include "game/tankContactInfo.h"

CannonBall::CannonBall(cAni::iAnimResManager &arm)
    : collisionEntity(*this), graphicEntity(*this, arm), cannon(NULL), orientation(0.f), speed(1.f), bExplode(false)
{
}

void CannonBall::step(float gameTime, float deltaTime)
{
    Point2f delta = dest_pos - pos;
    float len;
    if (!bExplode)
    {
        len = delta.DotProduct();
        if (len < 1)
        {
            bExplode = false;
            explodePos = pos;
            Explode(gameTime);
            return;
        }
    }
    else
    {
        bExplode = false;
        Explode(gameTime);
        return;
    }
    Point2f vel = delta.Normalize(min(sqrtf(len) / deltaTime, speed)); // 可能引发爆发事件
    collisionEntity.getBody().setVelocity(vel);
    collisionEntity.getBody().Update(deltaTime);
}

void CannonBall::setPostion(const Point2f &_pos, const Point2f &_dest_pos)
{
    pos = _pos;
    collisionEntity.resetTransform();
    dest_pos = _dest_pos;
}

void CannonBall::Explode(float gameTime)
{
    Explosive *e = new Explosive(graphicEntity.animResManager);
    Point2f delta = dest_pos - pos;
    float len = delta.DotProduct();
    if (len > 1)
    {
        delta = delta.Normalize(speed);// 可能引发爆发事件
    }
    e->speed = speed;
    e->orientation = orientation;
    e->init("data/tank/explode.xml", gameTime, cannon->explosive);
    e->setPostion(explodePos, explodePos + delta);
    EntityManager::getSingleton().attach(e);
    active = false;
}

Entity::iEntity* CannonBall::getEntityInterface(EntityInterfaceId id)
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

bool CannonBall::getStatus(EntityStatusId id, void *p)
{
    switch(id)
    {
    case ESI_Damage:
        assert(cannon);
        *(float *)p = cannon->fDamage;
        return true;
    case ESI_Penetrability:
        assert(cannon);
        *(int *)p = cannon->nPenetrability;
        return true;
    }

    assert(0 && "invalid");
    return false;
}

void CannonBall::CollisionEntity::onCollision(iCollisionEntity &o, const iContactInfo &ci)
{
    o, ci;
    getEntity().bExplode = true;
    const TankContactInfo &tci = *(const TankContactInfo *)&ci;
    if (tci.getNumber() > 0)
    {
        getEntity().explodePos = tci.getData()->m_xContacts[0];
    }
}

CannonBall::GraphicEntity::GraphicEntity(Entity &e, cAni::iAnimResManager &arm) : iGraphicEntity(e, arm)
{
    anim = iSystem::GetInstance()->createAnimation();
    anim->setAnimData(arm.getAnimData("data/tank/cannonball.xml"), 0);
}

CannonBall::GraphicEntity::~GraphicEntity()
{
    iSystem::GetInstance()->release(this->anim);
    this->anim = 0;
}

void CannonBall::GraphicEntity::render(float gameTime, float deltaTime)
{
    deltaTime;

    assert(anim);
    iRenderQueue::getSingleton().render(getEntity().pos, -getEntity().orientation, int(gameTime * 30), anim, GLI_Bullets);

//#ifdef _DEBUG
    //const Rectf &r = getEntity().collisionEntity.getBoundingBox();
    //iRenderQueue::getSingleton().render(Point2f(r.left, r.top), Point2f(r.right, r.top), ARGB(128, 0, 0, 255));
    //iRenderQueue::getSingleton().render(Point2f(r.right, r.top), Point2f(r.right, r.bottom), ARGB(128, 0, 0, 255));
    //iRenderQueue::getSingleton().render(Point2f(r.right, r.bottom), Point2f(r.left, r.bottom), ARGB(128, 0, 0, 255));
    //iRenderQueue::getSingleton().render(Point2f(r.left, r.bottom), Point2f(r.left, r.top), ARGB(128, 0, 0, 255));
//#endif
}
