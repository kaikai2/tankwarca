#ifndef CannonBall_H
#define CannonBall_H

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
struct Cannon;

class CannonBall : public Entity
{
public:
    enum EntityStatusIds
    {
        ESI_Damage = Entity::NumEntityStatusIds,
        ESI_Penetrability, // 穿透力

        NumEntityStatusIds,
    };

    CannonBall(cAni::iAnimResManager &arm);
    virtual ~CannonBall()
    {
    }
    void setPostion(const Point2f &_pos, const Point2f &_dest_pos);
    Point2f pos;
    Point2f dest_pos;
    float orientation;
    float speed;
    const Cannon *cannon; // 炮弹详细信息
    bool bExplode;

    Point2f explodePos;

    virtual void step(float gameTime, float deltaTime);

    void Explode(float gameTime);
    virtual iEntity* getEntityInterface(EntityInterfaceId id);
    virtual bool getStatus(EntityStatusId id, void *p);
    virtual EntityTypeId getTypeId() const
    {
        return ETI_CannonBall;
    }

    class CollisionEntity : public iCollisionEntity
    {
    public:
        CollisionEntity(Entity &e) : iCollisionEntity(e)
        {
            verlet::BoxShape *box = new verlet::BoxShape(body, Point2f(16 / 2, 5 / 2));
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
            body.Reset(getEntity().pos, getEntity().orientation, 0.1f);
            this->setboundingBox(box->GetBoundingBox() + getEntity().pos);
        }
        virtual unsigned long getCollisionCategory() const // 自己的类别标志位
        {
            return CC_Cannon;
        }
        virtual unsigned long getCollisionBits() const // 与哪种类发生碰撞
        {
            return CB_Cannon;
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
        CannonBall &getEntity()
        {
            return *(CannonBall*)&entity;
        }
        const CannonBall &getEntity() const
        {
            return *(const CannonBall*)&entity;
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
    protected:
        CannonBall &getEntity()
        {
            return *(CannonBall*)&entity;
        }
        iAnimation *anim;
    };
    CollisionEntity collisionEntity;
    GraphicEntity graphicEntity;
};


#endif//CannonBall_H
