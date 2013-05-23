#include "gameDef.h"
#include "radarMap.h"

Entity::iEntity *RadarMap::getEntityInterface(EntityInterfaceId id)
{
    switch(id)
    {
    case EII_GraphicEntity:
        return &graphicEntity;
    }
    return Entity::getEntityInterface(id);
}

void RadarMap::GraphicEntity::render(float gameTime, float deltaTime)
{
    Point2f pos;

    deltaTime;

    iRenderQueue::getSingleton().render(getEntity().pos, 0, int(gameTime * 60), anim, GLI_UI, true);

    for (vector<iAttacheeEntity*>::const_iterator ie = getEntity().entities.begin(); ie != getEntity().entities.end(); ++ie)
    {
        if (((iRadarEntity*)*ie)->getEntity().getStatus(ESI_Position, &pos))
        {
            pos *= 200.0f / 4000.0f;
            pos += getEntity().pos;
            iRenderQueue::getSingleton().render(pos + Point2f(-2, -2), pos + Point2f(2, 2), ARGB(255, 0, 0, 255), GLI_UI, true);
            iRenderQueue::getSingleton().render(pos + Point2f(2, -2), pos + Point2f(-2, 2), ARGB(255, 0, 0, 255), GLI_UI, true);
        }
    }
    Point2f viewerPos = iRenderQueue::getSingleton().getViewerPos() * 200.0f / 4000.0f + getEntity().pos;
    const Point2f &windowSize = iRenderQueue::getSingleton().getWindowSize();
    Point2f windowRange(windowSize.x / 4000.0f, windowSize.y / 4000.0f);
    windowRange *= 200.0f;

    iRenderQueue::getSingleton().render(viewerPos + Point2f(0, 0), viewerPos + Point2f(windowRange.x, 0), ARGB(255, 255, 255, 255), GLI_UI, true);
    iRenderQueue::getSingleton().render(viewerPos + Point2f(windowRange.x, windowRange.y), viewerPos + Point2f(windowRange.x, 0), ARGB(255, 255, 255, 255), GLI_UI, true);
    iRenderQueue::getSingleton().render(viewerPos + Point2f(windowRange.x, windowRange.y), viewerPos + Point2f(0, windowRange.y), ARGB(255, 255, 255, 255), GLI_UI, true);
    iRenderQueue::getSingleton().render(viewerPos + Point2f(0, 0), viewerPos + Point2f(0, windowRange.y), ARGB(255, 255, 255, 255), GLI_UI, true);
}