#ifndef GAME_RADAR_MAP_H
#define GAME_RADAR_MAP_H

#include "common/entity.h"
#include "common/graphicEntity.h"
#include "common/attachEntity.h"

class iRadarEntity : public iSingleAttacheeEntity
{
public:
    iRadarEntity(Entity &e) : iSingleAttacheeEntity(e)
    {
    }
    virtual ~iRadarEntity()
    {
    }
    DWORD color;
};

class RadarMap : public Entity, public MultiAttacher
{
public:
    RadarMap(cAni::iAnimResManager &arm) : graphicEntity(*this, arm)
    {

    }

    Point2f pos;

    virtual iEntity *getEntityInterface(EntityInterfaceId id);
    virtual void step(float gameTime, float deltaTime)
    {
        gameTime, deltaTime;
    }

    void init(const char *xml)
    {
        graphicEntity.init(xml);
    }
protected:
    class GraphicEntity : public iGraphicEntity
    {
    public:
        GraphicEntity(RadarMap &rm, cAni::iAnimResManager &arm) : iGraphicEntity(rm, arm)
        {
            anim = 0;
        }
        virtual ~GraphicEntity()
        {
            if (anim)
            {
                iSystem::GetInstance()->release(anim);
                anim = 0;
            }
        }
        void init(const char *xml)
        {
            if (!anim)
            {
                anim = iSystem::GetInstance()->createAnimation(1);
            }
            anim->setAnimData(animResManager.getAnimData(xml), 0);
        }
        virtual void render(float gameTime, float deltaTime);
    protected:
        const RadarMap &getEntity() const
        {
            return *(const RadarMap *)&entity;
        }
        iAnimation *anim;
    };

    GraphicEntity graphicEntity;
};

#endif//GAME_RADAR_MAP_H
