#include <hge.h>
#include "common/graphicEntity.h"
#include "common/entityManager.h"

#include "bullet.h"
#include "weapon.h"
#include "explosive.h"
//#include "game/mainGameState.h"
#include "game/tankContactInfo.h"

Bullet::Bullet(cAni::iAnimResManager &arm)
    : collisionEntity(*this), graphicEntity(*this, arm), gun(NULL), orientation(0.f), speed(1.f), bExplode(false)
{
}

void Bullet::step(float gameTime, float deltaTime)
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
    Point2f vel = delta.Normalize(min(sqrtf(len) / deltaTime, speed));// 可能引发爆发事件
    collisionEntity.getBody().setVelocity(vel);
    collisionEntity.getBody().Update(deltaTime);
}

void Bullet::setPostion(const Point2f &_pos, const Point2f &_dest_pos)
{
    pos = _pos;
    collisionEntity.resetTransform();
    dest_pos = _dest_pos;
}

void Bullet::Explode(float gameTime)
{
    Explosive *e = new Explosive(graphicEntity.animResManager);

    Point2f delta = dest_pos - pos;
    float len = delta.DotProduct();
    if (len > 1)
    {
        delta = delta.Normalize(speed);
    }
    e->orientation = orientation;
    e->init("data/tank/bulletexplode.xml", gameTime, gun->explosive);
    e->setPostion(explodePos, explodePos + delta);
    EntityManager::getSingleton().attach(e);

    active = false;
}

Entity::iEntity* Bullet::getEntityInterface(EntityInterfaceId id)
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

bool Bullet::getStatus(EntityStatusId id, void *p)
{
    switch(id)
    {
    case ESI_Damage:
        assert(gun);
        *(float *)p = gun->fDamage;
        return true;
    case ESI_Penetrability:
        assert(gun);
        *(int *)p = gun->nPenetrability;
        return true;
    }

    assert(0 && "invalid");
    return false;
}

void Bullet::CollisionEntity::onCollision(iCollisionEntity &o, const iContactInfo &ci)
{
    o, ci;
    getEntity().bExplode = true;
    const TankContactInfo &tci = *(const TankContactInfo *)&ci;
    if (tci.getNumber() > 0)
    {
        getEntity().explodePos = tci.getData()->m_xContacts[0];
    }
}

Bullet::GraphicEntity::GraphicEntity(Entity &e, cAni::iAnimResManager &arm) : iGraphicEntity(e, arm)
{
    anim = iSystem::GetInstance()->createAnimation();
    anim->setAnimData(arm.getAnimData("data/tank/bullet.xml"), 0);
}

Bullet::GraphicEntity::~GraphicEntity()
{
    iSystem::GetInstance()->release(this->anim);
    this->anim = 0;
}
bool Bullet::GraphicEntity::CheckEnd(float gameTime) const
{
    assert(anim);
    return anim->checkEnd(int(gameTime * 60));
}

void Bullet::GraphicEntity::render(float gameTime, float deltaTime)
{
    deltaTime;

    assert(anim);
    iRenderQueue::getSingleton().render(getEntity().pos, -getEntity().orientation, int(gameTime * 60), anim, GLI_Bullets);


//#ifdef _DEBUG
//    const Rectf &r = getEntity().collisionEntity.getBoundingBox();
//    iRenderQueue::getSingleton().render(Point2f(r.left, r.top), Point2f(r.right, r.top), ARGB(128, 0, 0, 255));
//    iRenderQueue::getSingleton().render(Point2f(r.right, r.top), Point2f(r.right, r.bottom), ARGB(128, 0, 0, 255));
//    iRenderQueue::getSingleton().render(Point2f(r.right, r.bottom), Point2f(r.left, r.bottom), ARGB(128, 0, 0, 255));
//    iRenderQueue::getSingleton().render(Point2f(r.left, r.bottom), Point2f(r.left, r.top), ARGB(128, 0, 0, 255));
//#endif

}

