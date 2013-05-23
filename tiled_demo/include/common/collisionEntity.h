#ifndef COMMON_COLLISION_ENTITY_H
#define COMMON_COLLISION_ENTITY_H

#include <vector>
#include "caRect.h"
#include "common/entity.h"

using std::vector;
using cAni::Rectf;

namespace verlet
{
    class iBody;
}

class iContactInfo
{
public:
    virtual void Update() = 0;
};
class iCollisionEntity : public Entity::iEntity
{
    friend class CollisionSpace;
public:
    iCollisionEntity(Entity &e) : Entity::iEntity(e), space(0), bSetBoundingBox(true)
    {
    }
    bool collisionEnabled() const
    {
        return bEnableCollision;
    }
    virtual unsigned long getCollisionCategory() const // �Լ�������־λ
    {
        return 0;
    }
    virtual unsigned long getCollisionBits() const // �������෢����ײ
    {
        return 0;
    }
    virtual unsigned long getCollisionBlockBits() const // ��ײ���Է����赲������
    {
        return getCollisionBits();
    }
    virtual void onCollision(iCollisionEntity &o, const iContactInfo &contactInfo)
    {
        o, contactInfo;
    }
    virtual verlet::iBody &getBody() = 0;
    virtual const verlet::iBody &getBody() const = 0;
    virtual void onAfterPhysics()
    {
        // update boundingBox here
    }
    void addIgnoredEntities(const iCollisionEntity &ie)
    {
        ignoredEntities.push_back(&ie);
    }
    const vector<const iCollisionEntity *> &getIgnoredEntities() const
    {
        return ignoredEntities;
    }
    const Rectf &getBoundingBox() const
    {
        return boundingBox;
    }
    bool isBoundingBoxChanged() const
    {
        return bSetBoundingBox;
    }
    void clearBoundingBoxChanged()
    {
        bSetBoundingBox = false;
    }
protected:
    void setboundingBox(const Rectf &rect)
    {
        if (!bSetBoundingBox && boundingBox == rect)
            return;

        boundingBox = rect;
        bSetBoundingBox = true;
    }
    bool bEnableCollision;
    bool bSetBoundingBox;
    Rectf boundingBox;
    vector<const iCollisionEntity *> ignoredEntities;
    CollisionSpace *space;
};

#endif//COMMON_COLLISION_ENTITY_H