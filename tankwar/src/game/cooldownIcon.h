#ifndef GAME_COOLDOWN_ICON_H
#define GAME_COOLDOWN_ICON_H

#include "common/graphicEntity.h"
#include "common/attachEntity.h"
#include "game/dataCurvedani.h"

class CooldownIcon : public Entity, public SingleAttacher
{
public:
    CooldownIcon(cAni::iAnimResManager &arm) : graphicEntity(*this, arm)
    {

    }

    Point2f pos;
    int statusId;
    int statusId_ammoAmount;

    virtual iEntity *getEntityInterface(EntityInterfaceId id);
    virtual void step(float gameTime, float deltaTime)
    {
        gameTime, deltaTime;
    }

    void init(const char *xml, const char *xmlMaskData)
    {
        graphicEntity.init(xml, xmlMaskData);
    }
protected:
    float getValue(float gameTime) const;
    size_t getAmmoAmount() const;

    class GraphicEntity : public iGraphicEntity
    {
    public:
        GraphicEntity(CooldownIcon &rm, cAni::iAnimResManager &arm) : iGraphicEntity(rm, arm)
        {
            anim = 0;
            maskdata = 0;
            dtRenderer = (DataTriangleRenderer *)iSystem::GetInstance()->getRenderer("DataTriangle");
        }
        virtual ~GraphicEntity()
        {
            if (anim)
            {
                iSystem::GetInstance()->release(anim);
                anim = 0;
            }
            if (maskdata)
            {
                iSystem::GetInstance()->release(maskdata);
                maskdata = 0;
            }
        }
        void init(const char *xml, const char *xmlMaskData)
        {
            if (!anim)
            {
                anim = iSystem::GetInstance()->createAnimation(1);
            }
            anim->setAnimData(animResManager.getAnimData(xml), 0);

            if (!maskdata)
                maskdata = iSystem::GetInstance()->createAnimation(1);
            maskdata->setAnimData(animResManager.getAnimData(xmlMaskData), 0);
        }
        virtual void render(float gameTime, float deltaTime);
    protected:
        const CooldownIcon &getEntity() const
        {
            return *(const CooldownIcon *)&entity;
        }
        iAnimation *anim;
        iAnimation *maskdata;
        DataTriangleRenderer *dtRenderer;
    };

    GraphicEntity graphicEntity;
};

#endif//GAME_COOLDOWN_ICON_H