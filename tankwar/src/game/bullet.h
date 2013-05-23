#ifndef Bullet_H
#define Bullet_H

#include "curvedani.h"
#include "caPoint2d.h"
#include "verlet/verlet.h"

#include "common/entity.h"
#include "common/collisionEntity.h"
#include "common/graphicEntity.h"

#include "collisionDef.h"
#include "gameDef.h"

using cAni::iSystem;
using cAni::iAnimation;
using cAni::Point2f;
struct Gun;

class Bullet : public Entity
{
public:
    enum EntityStatusIds
    {
        ESI_Damage = Entity::NumEntityStatusIds,
        ESI_Penetrability,

        NumEntityStatusIds,
    };
    Bullet(cAni::iAnimResManager &arm);
    void setPostion(const Point2f &_pos, const Point2f &_dest_pos);
    Point2f pos;
    Point2f dest_pos;
    float orientation;
    float speed;
    const Gun *gun; // 炮弹详细信息
    bool bExplode;

    Point2f explodePos; // 爆炸位置

    virtual void step(float gameTime, float deltaTime);

    void Explode(float gameTime);
    virtual iEntity* getEntityInterface(EntityInterfaceId id);
    virtual bool getStatus(EntityStatusId id, void *p);
    virtual EntityTypeId getTypeId() const
    {
        return ETI_Bullet;
    }

    class CollisionEntity : public iCollisionEntity
    {
    public:
        CollisionEntity(Entity &e) : iCollisionEntity(e)
        {
            verlet::BoxShape *box = new verlet::BoxShape(body, Point2f(16 / 2, .5 / 2));
            body.SetShape(box);
            resetTransform();
        }
        virtual ~CollisionEntity()
        {
            delete body.GetShape();
            body.SetShape(0);
        }
        void resetTransform()
        {
            verlet::BoxShape *box = (verlet::BoxShape *)body.GetShape();
            assert(box);
            body.Reset(getEntity().pos, getEntity().orientation, 0.01f);
            this->setboundingBox(box->GetBoundingBox() + getEntity().pos);
        }
        virtual unsigned long getCollisionCategory() const // 自己的类别标志位
        {
            return CC_Bullet;
        }
        virtual unsigned long getCollisionBits() const // 与哪种类发生碰撞
        {
            return CB_Bullet;
        }
        virtual void onCollision(iCollisionEntity &o, const iContactInfo &ci);
        virtual verlet::iBody &getBody()
        {
            return body;
        }
        virtual const verlet::iBody &getBody() const
        {
            return body;
        }
        virtual void onAfterPhysics()
        {
            getEntity().pos = body.getPosition();
            getEntity().orientation = body.getOrientation();

            verlet::BoxShape *box = (verlet::BoxShape *)body.GetShape();
            assert(box);

            this->setboundingBox(box->GetBoundingBox() + getEntity().pos);
        }
    protected:
        Bullet &getEntity()
        {
            return *(Bullet*)&entity;
        }
        const Bullet &getEntity() const
        {
            return *(const Bullet*)&entity;
        }
        verlet::Body body;
    };
protected:
    class GraphicEntity : public iGraphicEntity
    {
    public:
        GraphicEntity(Entity &e, cAni::iAnimResManager &arm);
        virtual ~GraphicEntity();
        virtual void render(float gameTime, float deltaTime);
        void Explode(float gameTime);
        bool CheckEnd(float gameTime) const;
    protected:
        Bullet &getEntity()
        {
            return *(Bullet*)&entity;
        }
        iAnimation *anim;
    };
    CollisionEntity collisionEntity;
    GraphicEntity graphicEntity;
};


#endif//Bullet_H
