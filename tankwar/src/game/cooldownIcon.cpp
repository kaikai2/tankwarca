#include <hgefont.h>
#include <vector>
#include "game/dataCurvedAni.h"
#include "game/mainGameState.h"

#include "cooldownIcon.h"

#include "gameDef.h"
#include "tank.h"

using std::vector;


Entity::iEntity *CooldownIcon::getEntityInterface(EntityInterfaceId id)
{
    switch(id)
    {
    case EII_GraphicEntity:
        return &graphicEntity;
    }
    return Entity::getEntityInterface(id);
}
float CooldownIcon::getValue(float gameTime) const
{
    if (entity)
    {
        Entity &e = entity->getEntity(); // get tank
        float value = gameTime;
        e.getStatus(statusId, &value);
        return value;
    }
    return 1.0f;
}

size_t CooldownIcon::getAmmoAmount() const
{
    if (entity)
    {
        Entity &e = entity->getEntity();
        size_t value;
        e.getStatus(statusId_ammoAmount, &value);
        return value;
    }
    return 0;
}

void CooldownIcon::GraphicEntity::render(float gameTime, float deltaTime)
{
    const Point2f &pos = getEntity().pos;

    deltaTime;

    if (!anim || !maskdata)
        return;
    
    int frameTime = int(gameTime * 60);
    iRenderQueue::getSingleton().render(pos, 0, frameTime, anim, GLI_UI, true);

    int ft = int(getEntity().getValue(gameTime) * 80);
    dataClipState ds;
    maskdata->render(ft, &ds);

    if (dtRenderer)
    {
        dtRenderer->render(pos);
    }

    char buf[32];
    sprintf(buf, "%d", getEntity().getAmmoAmount());
    iRenderQueue::getSingleton().render(pos + Point2f(30, -30), MainGameState::getSingleton().getFont(), buf, HGETEXT_RIGHT, ARGB(255, 128, 255, 128), GLI_UI, true);
}