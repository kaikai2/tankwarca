#ifndef ENTITY_H
#define ENTITY_H

#include "common/utils.h"

#pragma warning(disable: 4355 4250)


class Entity : public RefCountedObj
{
public:
    enum EntityInterfaceIds
    {
        EII_CollisionEntity,
        EII_ControlEntity,
        EII_GraphicEntity,
        EII_ViewerEntity,

        NumEntityInterfaceIds,
    };
    typedef int EntityInterfaceId;
    // allow to extend, so the type should not be EntityStatusIds
    enum EntityStatusIds
    {
        ESI_Position,
        ESI_Orientation,
        ESI_Velocity,

        NumEntityStatusIds,
    };
    // instead, let's use int
    typedef int EntityStatusId;
    typedef int EntityTypeId;

    // basic interface of entity
    class iEntity : public NoCopy
    {
    public:
        iEntity(Entity &e) : entity(e)
        {
        }
        virtual ~iEntity()
        {
        }
        Entity &getEntity()
        {
            return entity;
        }
        const Entity &getEntity() const
        {
            return entity;
        }
    protected:
        Entity &entity;
    };

    Entity() : active(true)
    {
    }
    virtual ~Entity()
    {
    }
    virtual void step(float gameTime, float deltaTime)
    {
        gameTime, deltaTime;
    }
    virtual bool getStatus(EntityStatusId id, void *p)
    {
        id, p;
        return false;
    }
    virtual EntityTypeId getTypeId() const
    {
        return 0;
    }
    bool isActive() const
    {
        return active;
    }
    bool canRelease() const
    {
        return !isActive() && count == 0;
    }
    virtual iEntity* getEntityInterface(EntityInterfaceId id)
    {
        id;
        return 0;
    }
protected:
    bool active;
};

typedef Ref<Entity> rEntity;

#endif