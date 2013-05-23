#include <hge.h>
#include "weapon.h"
#include "explosive.h"
#include "common/graphicEntity.h"
#include "common/entityManager.h"

Explosive::Explosive(cAni::iAnimResManager &arm)
    : collisionEntity(*this), graphicEntity(*this, arm), explosiveInfo(NULL), orientation(0.f), speed(1.f)
{
}

void Explosive::step(float gameTime, float deltaTime)
{
    gameTime;
    if (graphicEntity.CheckEnd(gameTime))
    {
        active = false;
    }
    Point2f delta = dest_pos - pos;
    float len = delta.DotProduct();
    if (len < 1)
    {
        return;
    }
    len = 0;
    Point2f vel = delta.Normalize(min(sqrtf(len) / deltaTime, speed));// 可能引发爆发事件
    collisionEntity.getBody().setVelocity(vel);
    collisionEntity.getBody().Update(deltaTime);
}

void Explosive::setPostion(const Point2f &_pos, const Point2f &_dest_pos)
{
    pos = _pos;
    if (explosiveInfo)
        collisionEntity.resetTransform();
    dest_pos = _dest_pos;
}

Entity::iEntity* Explosive::getEntityInterface(EntityInterfaceId id)
{
    switch(id)
    {
    case EII_CollisionEntity:
        if (explosiveInfo)
            return &collisionEntity;
        break;
    case EII_GraphicEntity:
        return &graphicEntity;
    }

    return Entity::getEntityInterface(id);
}

void Explosive::getDamage(Point2f &op, float &fDamage, int &nPenetrability) const
{
    if (explosiveInfo)
    {
        float distance = (op - pos).Length();
        float damageScale = clamp(1.f - distance / explosiveInfo->fDamageRange, 0.f, 1.f);
        fDamage = explosiveInfo->fDamage * damageScale;
        nPenetrability = clamp(int(explosiveInfo->nPenetrability * damageScale), 0, 100);
    }
    else
    {
        fDamage = 0.f;
        nPenetrability = 0;
    }
}

bool Explosive::getStatus(EntityStatusId id, void *p)
{
    switch(id)
    {
    case ESI_Position:
        *(Point2f *)p = this->pos;
        return true;
    case ESI_Damage:
        *(float *)p = explosiveInfo ? explosiveInfo->fDamage : 0.f;
        return true;
    case ESI_Penetrability:
        *(int *)p = explosiveInfo ? explosiveInfo->nPenetrability : 0;
        return true;
    }

    assert(0 && "invalid");
    return false;
}

void Explosive::CollisionEntity::onCollision(iCollisionEntity &o, const iContactInfo &ci)
{
    ci;
    this->addIgnoredEntities(o);
}

Explosive::GraphicEntity::GraphicEntity(Entity &e, cAni::iAnimResManager &arm) : iGraphicEntity(e, arm), anim(0)
{
}

Explosive::GraphicEntity::~GraphicEntity()
{
    iSystem::GetInstance()->release(this->anim);
    this->anim = 0;
}

bool Explosive::GraphicEntity::CheckEnd(float gameTime) const
{
    assert(anim);
    return anim->checkEnd(int(gameTime * 60));
}

void Explosive::GraphicEntity::init(const char *xml, float gameTime)
{
    if (!anim)
    {
        anim = iSystem::GetInstance()->createAnimation();
    }
    anim->setAnimData(animResManager.getAnimData(xml), 0);
    anim->startAnim(int(gameTime * 60));
    anim->setAnimLoop(false);
}

void Explosive::GraphicEntity::render(float gameTime, float deltaTime)
{
    deltaTime;

    assert(anim);
    iRenderQueue::getSingleton().render(getEntity().pos, -getEntity().orientation, int(gameTime * 60), anim, GLI_Bullets);


//#ifdef _DEBUG
    //const Rectf &r = getEntity().collisionEntity.getBoundingBox();
    //iRenderQueue::getSingleton().render(Point2f(r.leftTop.x, r.leftTop.y), Point2f(r.rightBottom.x, r.leftTop.y), ARGB(128, 0, 0, 255));
    //iRenderQueue::getSingleton().render(Point2f(r.rightBottom.x, r.leftTop.y), Point2f(r.rightBottom.x, r.rightBottom.y), ARGB(128, 0, 0, 255));
    //iRenderQueue::getSingleton().render(Point2f(r.rightBottom.x, r.rightBottom.y), Point2f(r.leftTop.x, r.rightBottom.y), ARGB(128, 0, 0, 255));
    //iRenderQueue::getSingleton().render(Point2f(r.leftTop.x, r.rightBottom.y), Point2f(r.leftTop.x, r.leftTop.y), ARGB(128, 0, 0, 255));
//#endif

}

