#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

#include <vector>
#include "caPoint2d.h"
#include "common/entity.h"
#include "common/collisionEntity.h"
#include "common/graphicEntity.h"
#include "common/utils.h"
#include "common/collisionSpace.h"

using std::vector;
using cAni::Point2f;

class CollisionChecker
{
public:
    virtual bool checkCollision(iCollisionEntity *a, iCollisionEntity *b, iContactInfo &contactInfo) = 0;
};

class Culler;

class EntityFilter;


class EntityManager : public Singleton<EntityManager>
{
public:
    typedef vector<Entity *> Entities;
    EntityManager() : bModifyLock(false), collisionChecker(0), contactInfo(0)
        , collisionSpace(5)
    {
        Rectf r;
        r.leftTop = Point2f(0, 0);
        r.rightBottom = Point2f(800, 600);
        collisionSpace.setRange(r);
    }
    virtual ~EntityManager()
    {
        clear();
    }
    void clear();
    void step(float gameTime, float deltaTime);
    void render(float gameTime, float deltaTime);
    void attach(Entity *entity);
    void dettach(Entity *entity);
    void dettach(vector<Entity *> &entities);

    void getEntities(vector<Entity *> &entities, const EntityFilter &filter);

    void setPlayRange(Rectf &r)
    {
        collisionSpace.setRange(r);
        collisionSpace.update();
    }
    void checkCollision();
    void setCollisionChecker(CollisionChecker *checker)
    {
        collisionChecker = checker;
    }
    void setContactInfoBuffer(iContactInfo &ci)
    {
        contactInfo = &ci;
    }
    size_t getCount() const
    {
        return em_entities.size();
    }
protected:
    iContactInfo &getContactInfoBuffer()
    {
        assert(contactInfo);
        return *contactInfo;
    }
    CollisionChecker *collisionChecker;

    Entities newComingEntities;
    /* 
    vc2005 ide bug, 怀疑太多地方使用名字：entities，在watch内出现此变量无法正确显示
    改为其他名字后回复正常：
    */
    Entities em_entities;

    bool bModifyLock;

    iContactInfo *contactInfo;
    CollisionSpace collisionSpace;
};

#endif