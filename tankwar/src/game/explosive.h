#ifndef Explosive_H
#define Explosive_H

#include "curvedani.h"
#include "caPoint2d.h"
#include "common/entity.h"
#include "common/collisionEntity.h"
#include "common/graphicEntity.h"
#include "verlet/verlet.h"

#include "collisionDef.h"
#include "gameDef.h"

using cAni::iSystem;
using cAni::iAnimation;
using cAni::Point2f;
struct Gun;

class Explosive : public Entity
{
public:
    enum EntityStatusIds
    {
        ESI_Damage = Entity::NumEntityStatusIds,
        ESI_Penetrability,

        NumEntityStatusIds,
    };
    Explosive(cAni::iAnimResManager &arm);
    void setPostion(const Point2f &_pos, const Point2f &_dest_pos);
    void init(const char *xml, float gameTime, const ExplosiveInfo *explosiveInfo)
    {
        graphicEntity.init(xml, gameTime);
        this->explosiveInfo = explosiveInfo;
        if (explosiveInfo)
        {
            collisionEntity.init(explosiveInfo->fDamageRange);
        }
    }
    void getDamage(Point2f &op, float &fDamage, int &nPenetrability) const;
    Point2f pos;
    Point2f dest_pos;
    float orientation;
    float speed;
    const ExplosiveInfo *explosiveInfo; // 炮弹详细信息

    virtual void step(float gameTime, float deltaTime);

    virtual iEntity* getEntityInterface(EntityInterfaceId id);
    virtual bool getStatus(EntityStatusId id, void *p);
    virtual EntityTypeId getTypeId() const
    {
        return ETI_Explosive;
    }

    class CollisionEntity : public iCollisionEntity
    {
    public:
        CollisionEntity(Entity &e) : iCollisionEntity(e)
        {
        }
        virtual ~CollisionEntity()
        {
            delete body.GetShape();
            body.SetShape(0);
        }
        void init(float radius)
        {
            if (body.GetShape())
            {
                delete body.GetShape();
            }
            verlet::iShape *shape = new verlet::CircleShape(body, radius);
            body.SetShape(shape);
            resetTransform();
        }
        void resetTransform()
        {
            verlet::iShape *shape = body.GetShape();
            assert(shape);
            body.Reset(getEntity().pos, getEntity().orientation, 0.01f);
            this->setboundingBox(shape->GetBoundingBox() + getEntity().pos);
        }
        virtual unsigned long getCollisionCategory() const // 自己的类别标志位
        {
            return CC_Explosive;
        }
        virtual unsigned long getCollisionBits() const // 与哪种类发生碰撞
        {
            return CB_Explosive;
        }
        virtual unsigned long getCollisionBlockBits() const
        {
            return CBB_Explosive;
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

            verlet::iShape *shape = body.GetShape();
            assert(shape);

            this->setboundingBox(shape->GetBoundingBox() + getEntity().pos);
        }
    protected:
        Explosive &getEntity()
        {
            return *(Explosive*)&entity;
        }
        const Explosive &getEntity() const
        {
            return *(const Explosive*)&entity;
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
        void init(const char *xml, float gameTime);
    protected:
        Explosive &getEntity()
        {
            return *(Explosive*)&entity;
        }
        iAnimation *anim;
    };
    CollisionEntity collisionEntity;
    GraphicEntity graphicEntity;
};


#endif//Explosive_H
