#ifndef GAME_ITEM_H
#define GAME_ITEM_H

#include "curvedani.h"
#include "caPoint2d.h"

#include "common/entity.h"
#include "common/graphicEntity.h"
#include "common/collisionEntity.h"

#include "collisionDef.h"
#include "gameDef.h"
#include "tank.h"

using cAni::Point2f;

class Item : public Entity, public InstanceCount<Item>
{
public:
    Item(cAni::iAnimResManager &arm);
    virtual void step(float gameTime, float deltaTime)
    {
        gameTime, deltaTime;
    }
    virtual bool getStatus(EntityStatusId id, void *p)
    {
        switch(id)
        {
        case Entity::ESI_Position:
            *(Point2f *)p = pos;
            return true;
        }
        return false;
    }
    virtual iEntity* getEntityInterface(EntityInterfaceId id)
    {
        switch(id)
        {
        case EII_GraphicEntity:
            return &graphicEntity;
        case EII_CollisionEntity:
            return &collisionEntity;
        }
        return 0;
    }
    virtual EntityTypeId getTypeId() const
    {
        return ETI_Item;
    }

    virtual void pickedBy(Tank &tank) = 0;
    void setPosition(const Point2f &pos)
    {
        this->pos = pos;
        collisionEntity.resetTransform();
    }
protected:
    Point2f pos;
    class GraphicEntity : public iGraphicEntity
    {
    public:
        GraphicEntity(Item &item, cAni::iAnimResManager &arm);
        ~GraphicEntity();
        void init(const char *xml);
        virtual void render(float gameTime, float deltaTime);
    protected:
        const Item &getEntity() const
        {
            return *(const Item*)&entity;
        }
        cAni::iAnimation *anim;
    };
    class CollisionEntity : public iCollisionEntity
    {
    public:
        CollisionEntity(Item &item) : iCollisionEntity(item)
        {
            verlet::BoxShape *box = new verlet::BoxShape(body, Point2f(30 / 2, 30 / 2));
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
            body.Reset(getEntity().pos, 0, 0.01f);
            this->setboundingBox(box->GetBoundingBox() + getEntity().pos);
        }
        virtual unsigned long getCollisionCategory() const // 自己的类别标志位
        {
            return CC_Item;
        }
        virtual unsigned long getCollisionBits() const // 与哪种类发生碰撞
        {
            return CB_Item;
        }
        virtual unsigned long getCollisionBlockBits() const // 碰撞可以发生阻挡的类型
        {
            return CBB_Item;
        }

        virtual void onCollision(iCollisionEntity &o, const iContactInfo &contactInfo)
        {
            o, contactInfo;
            switch(o.getCollisionCategory())
            {
            case CC_Tank:
                {
                    Tank &tank = static_cast<Tank &>(o.getEntity());
                    getEntity().pickedBy(tank);
                }
                break;
            }
        }
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
            // update boundingBox here
        }
    protected:
        Item &getEntity()
        {
            return *(Item*)&entity;
        }
        const Item &getEntity() const
        {
            return *(const Item*)&entity;
        }
        verlet::Body body;
    };
    GraphicEntity graphicEntity;
    CollisionEntity collisionEntity;
};

#endif//GAME_ITEM_H