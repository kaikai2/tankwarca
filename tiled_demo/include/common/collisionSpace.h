#ifndef COMMON_COLLISION_SPACE_H
#define COMMON_COLLISION_SPACE_H

#include <vector>
#include <list>
#include "caPoint2d.h"
#include "common/entity.h"
#include "common/collisionEntity.h"
#include "common/graphicEntity.h"
#include "common/quadSpace.h"
#include "common/utils.h"

#define USE_RESPLIT

using std::vector;
using std::list;
using cAni::Point2f;

class CollisionChecker;
class EntityFilter;
class Culler : public NoCopy
{
public:
    Culler(vector<Entity *> &_entities) : entities(_entities)
    {
    }
    void cull(CollisionSpace &space, const EntityFilter &ef);

    Rectf viewer;
    vector<Entity *> &entities;
};

class EntityFilter : public NoCopy
{
public:
    virtual bool filtrate(Entity &e) const = 0;
    virtual const Rectf &getBoundingBox() const = 0;
};

class CollisionSpace : public QuadSpace
{
public:
    typedef vector<iCollisionEntity *> CollisionEntities;
    typedef vector<iCollisionEntity *> NewCollisionEntities;
    CollisionSpace(int depth, CollisionSpace* _parent = 0) : QuadSpace(depth, _parent)
    {
        if (depth > 0)
        {
            child = (CollisionSpace *)allocBuffer(sizeof(CollisionSpace));
            for (int i = 0; i < NumSubSpaces; i++)
                new(&getChild(i)) CollisionSpace(depth - 1, this);
        }
    }
    virtual ~CollisionSpace()
    {
        if (child)
        {
            for (int i = 0; i < NumSubSpaces; i++)
                getChild(i).~CollisionSpace();
        }
    }
    CollisionSpace &getChild(size_t id)
    {
        return ((CollisionSpace*)child)[id];
    }
    const CollisionSpace &getChild(size_t id) const
    {
        return ((const CollisionSpace*)child)[id];
    }
    CollisionSpace &getParent()
    {
        return *(CollisionSpace*)parent;
    }
    const CollisionSpace &getParent() const
    {
        return *(const CollisionSpace*)parent;
    }
    void update();
#ifdef USE_RESPLIT
    void splitNewNodes();
#else
    /// @return count of newEntities (include child nodes' newEntities)
    size_t updateInner();
#endif
    bool checkUpdateSplit() const
    {
        for (CollisionEntities::const_iterator ie = entities.begin(); ie != entities.end(); ++ie)
        {
            iCollisionEntity *ce = *ie;
            const Rectf &r = ce->getBoundingBox();
            if (!isContained(r))
            {
                if (parent)
                    return false;

                if (child)
                {
                    if (r.rightBottom.x <= center.x && r.rightBottom.y <= center.y)
                        return false;
                    if (r.rightBottom.x <= center.x && r.leftTop.y > center.y)
                        return false;
                    if (r.leftTop.x > center.x && r.rightBottom.y <= center.y)
                        return false;
                    if (r.leftTop.x > center.x && r.leftTop.y > center.y)
                        return false;
                }
            }
        }
        return true;
    }
    bool checkUpdate() const
    {
        if (child)
        {
            for (size_t i = 0; i < NumSubSpaces; i++)
            {
                if (!getChild(i).checkUpdate())
                    return false;
            }
        }
        return checkUpdateSplit() && newEntities.empty();
    }
    void addEntity(iCollisionEntity *e)
    {
        assert(e && e->space == 0);
        e->space = this;
        newEntities.push_back(e);
    }
    static void removeEntity(iCollisionEntity *e);
    void removeEntityAll(iCollisionEntity *e);
    size_t checkInnerCollision(CollisionChecker &collisionChecker, iContactInfo &contactInfo);
    size_t checkCollision(iCollisionEntity *e, CollisionChecker &collisionChecker, iContactInfo &contactInfo);

    void renderDebug(float alpha);

    void cull(Culler &culler, const EntityFilter &ef) const;
protected:

    NewCollisionEntities newEntities;
    CollisionEntities entities;
};
#endif // COMMON_COLLISION_SPACE_H
