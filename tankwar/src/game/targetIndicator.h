#ifndef GAME_TARGET_INDICATOR_H
#define GAME_TARGET_INDICATOR_H

#include "common/utils.h"
#include "common/entity.h"
#include "common/graphicEntity.h"
#include "common/attachEntity.h"

class iTargetEntity : public iSingleAttacheeEntity
{
public:
    iTargetEntity(Entity &e) : iSingleAttacheeEntity(e)
    {
    }
    virtual ~iTargetEntity()
    {
    }
};

class TargetIndicator : public Entity, public Singleton<TargetIndicator>, public MultiAttacher
{
public:
    TargetIndicator(cAni::iAnimResManager &arm) : graphicEntity(*this, arm)
    {
    }

    void init(const char *xml)
    {
        graphicEntity.init(xml);
    }
    virtual void step(float gameTime, float deltaTime)
    {
        gameTime, deltaTime;
    }
    virtual iEntity *getEntityInterface(EntityInterfaceId id);
protected:
    class GraphicEntity : public iGraphicEntity
    {
    public:
        GraphicEntity(TargetIndicator &ti, cAni::iAnimResManager &arm) : iGraphicEntity(ti, arm)
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
        TargetIndicator &getEntity()
        {
            return *(TargetIndicator *)&entity;
        }
        iAnimation *anim;
    };

    GraphicEntity graphicEntity;
};

#endif//GAME_TARGET_INDICATOR_H