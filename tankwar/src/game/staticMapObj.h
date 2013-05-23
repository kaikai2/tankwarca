#ifndef GAME_STATIC_MAP_OBJECT_H
#define GAME_STATIC_MAP_OBJECT_H

#include <cassert>
#include <vector>
#include "curvedani.h"
#include "caPoint2d.h"
#include "verlet/verlet.h"

#include "common/entity.h"
#include "common/collisionEntity.h"
#include "common/graphicEntity.h"

#include "collisionDef.h"
#include "gameDef.h"

using std::vector;
using cAni::Point2f;

class StaticMapObj : public Entity
{
public:
    StaticMapObj(cAni::iAnimResManager &arm) : collisionEntity(*this), graphicEntity(*this, arm), orientation(0)
    {
    }
    virtual ~StaticMapObj()
    {
    }
    void init(const char *anim, float w, float h, float ori)
    {
        collisionEntity.init(w, h, ori);
        graphicEntity.init(anim);
    }
    virtual void step(float gameTime, float deltaTime)
    {
        gameTime;
        Point2f vel = collisionEntity.getBody().getVelocity();
        //vel.Normalise();
        //vel * 0.01f;
        vel /= -deltaTime;
        collisionEntity.getBody().AddForce(vel);
        float avel = collisionEntity.getBody().getAngVelocity();
        if (avel > 1e-15 || avel < -1e-15)
        {
            avel *= 200;
            avel /= -deltaTime;
            collisionEntity.getBody().AddTorque(avel);
        }
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
    virtual EntityTypeId getTypeId() const
    {
        return ETI_Box;
    }

    void getBoundingVertices(vector<Point2f> &vertices) const
    {
        const verlet::BoxShape *shape = (const verlet::BoxShape *)collisionEntity.getBody().GetShape();
        const Point2f &dir0 = shape->GetDir(0);
        const Point2f &dir1 = shape->GetDir(1);
        const Point2f &extents = shape->GetExtents();
        vertices.push_back(pos + dir0 * extents.x + dir1 * extents.y);
        vertices.push_back(pos - dir0 * extents.x + dir1 * extents.y);
        vertices.push_back(pos - dir0 * extents.x - dir1 * extents.y);
        vertices.push_back(pos + dir0 * extents.x - dir1 * extents.y);
    }
protected:
    virtual size_t getRenderLayerId() const
    {
        return GLI_MapObj1;
    }
    virtual unsigned long getCollisionCategory() const // 自己的类别标志位
    {
        return CC_Box;
    }
    virtual unsigned long getCollisionBits() const // 与哪种类发生碰撞
    {
        return CB_Box;
    }
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
        void init(float w, float h, float ori)
        {
            if (body.GetShape())
                delete body.GetShape();
            verlet::BoxShape *box = new verlet::BoxShape(body, Point2f(w / 2, h / 2));
            body.SetShape(box);
            getEntity().orientation = ori;
            resetTransform();
        }
        void resetTransform()
        {
            verlet::BoxShape *box = (verlet::BoxShape *)body.GetShape();
            assert(box);
            body.Reset(getEntity().pos, getEntity().orientation);
            this->setboundingBox(box->GetBoundingBox() + getEntity().pos);
        }
        virtual unsigned long getCollisionCategory() const // 自己的类别标志位
        {
            return getEntity().getCollisionCategory();
        }
        virtual unsigned long getCollisionBits() const // 与哪种类发生碰撞
        {
            return getEntity().getCollisionBits();
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
        StaticMapObj &getEntity()
        {
            return *(StaticMapObj*)&entity;
        }
        const StaticMapObj &getEntity() const
        {
            return *(const StaticMapObj*)&entity;
        }
        verlet::StaticBody body;
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
            animShape->startAnim(rand() % 90);
        }
        virtual void render(float gameTime, float deltaTime);
    protected:
        StaticMapObj &getEntity()
        {
            return *(StaticMapObj*)&entity;
        }
        cAni::iAnimation *animShape;
    };
    CollisionEntity collisionEntity;
    GraphicEntity graphicEntity;
};

class Wetland : public StaticMapObj
{
public:
    Wetland(cAni::iAnimResManager &arm) : StaticMapObj(arm)
    {
    }
    virtual EntityTypeId getTypeId() const
    {
        return ETI_Wetland;
    }
protected:
    virtual unsigned long getCollisionCategory() const // 自己的类别标志位
    {
        return CC_Wetland;
    }
    virtual unsigned long getCollisionBits() const // 与哪种类发生碰撞
    {
        return CB_Wetland;
    }
    virtual size_t getRenderLayerId() const
    {
        return GLI_MapObj0;
    }
};

#endif//GAME_STATIC_MAP_OBJECT_H
