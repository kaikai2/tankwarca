#ifndef GAME_MAP_OBJECT_H
#define GAME_MAP_OBJECT_H

#include <cassert>
#include <vector>
#include "curvedani.h"
#include "caPoint2d.h"
#include "verlet/verlet.h"

#include "common/entity.h"
#include "common/collisionEntity.h"
#include "common/graphicEntity.h"

#include "collisionDef.h"

using std::vector;
using cAni::Point2f;

class MapObj : public Entity
{
public:
    MapObj(cAni::iAnimResManager &arm) : collisionEntity(*this), graphicEntity(*this, arm), orientation(0)
    {
    }
    virtual ~MapObj()
    {
    }
    void init(const char *anim, float w, float h)
    {
        collisionEntity.init(w, h);
        graphicEntity.init(anim);
    }
    virtual void step(float gameTime, float deltaTime)
    {
        gameTime;
        Point2f vel = collisionEntity.getBody().getVelocity();
        collisionEntity.getBody().setVelocity(vel * 0.8f);
        float avel = collisionEntity.getBody().getAngVelocity();
        collisionEntity.getBody().setAngVelocity(avel * 0.8f);
        collisionEntity.getBody().Update(deltaTime);
    }
    void setPosition(const Point2f &_pos)   
    {
        pos = _pos;
        this->collisionEntity.resetTransform();
    }
    Point2f pos;
    float orientation;

    virtual iEntity* getEntityInterface(EntityInterfaceId id);
protected:
    class CollisionEntity : public iCollisionEntity
    {
    public:
        CollisionEntity(Entity &e) : iCollisionEntity(e)
        {
            verlet::BoxShape *box = new verlet::BoxShape(body, Point2f(32 / 2, 32 / 2));
            body.SetShape(box);
            resetTransform();
        }
        virtual ~CollisionEntity()
        {
            delete body.GetShape();
            body.SetShape(0);
        }
        void init(float w, float h)
        {
            if (body.GetShape())
                delete body.GetShape();
            verlet::BoxShape *box = new verlet::BoxShape(body, Point2f(w / 2, h / 2));
            body.SetShape(box);
            resetTransform();
        }
        void resetTransform()
        {
            verlet::BoxShape *box = (verlet::BoxShape *)body.GetShape();
            assert(box);
            const Point2f &extents = box->GetExtents();
            body.Reset(getEntity().pos, getEntity().orientation, extents.x * extents.y * 0.001f);
            this->setboundingBox(box->GetBoundingBox() + getEntity().pos);
        }
        virtual unsigned long getCollisionCategory() const // 自己的类别标志位
        {
            return CC_Box;
        }
        virtual unsigned long getCollisionBits() const // 与哪种类发生碰撞
        {
            return CB_Box;
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
            const Point2f &pos = body.getPosition();
            assert((getEntity().pos - pos).Length() < 1e2);
            getEntity().pos = pos;
            getEntity().orientation = body.getOrientation();
            verlet::BoxShape *box = (verlet::BoxShape *)body.GetShape();
            assert(box);
            this->setboundingBox(box->GetBoundingBox() + getEntity().pos);
        }

    protected:
        MapObj &getEntity()
        {
            return *(MapObj*)&entity;
        }
        verlet::Body body;
    };
    class GraphicEntity : public iGraphicEntity
    {
    public:
        GraphicEntity(Entity &e, cAni::iAnimResManager &arm) : iGraphicEntity(e, arm), animShape(0)
        {
        }
        virtual ~GraphicEntity()
        {
            if (!animShape)
            {
                cAni::iSystem::GetInstance()->release(animShape);
                animShape = 0;
            }
        }
        void init(const char *anim)
        {
            if (!animShape)
            {
                animShape = cAni::iSystem::GetInstance()->createAnimation();
            }
            animShape->setAnimData(animResManager.getAnimData(anim), 0);
        }
        virtual void render(float gameTime, float deltaTime);
    protected:
        MapObj &getEntity()
        {
            return *(MapObj*)&entity;
        }
        cAni::iAnimation *animShape;
    };
    CollisionEntity collisionEntity;
    GraphicEntity graphicEntity;
};

#endif//GAME_MAP_OBJECT_H
