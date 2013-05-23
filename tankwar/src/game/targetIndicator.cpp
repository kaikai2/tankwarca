#include <caRect.h>
#include "targetIndicator.h"
#include "gameDef.h"

using cAni::Rectf;

Entity::iEntity *TargetIndicator::getEntityInterface(EntityInterfaceId id)
{
    switch(id)
    {
    case EII_GraphicEntity:
        return &this->graphicEntity;
    }

    return Entity::getEntityInterface(id);
}

void TargetIndicator::GraphicEntity::render(float gameTime, float deltaTime)
{
    int frameTime = int(gameTime * 60);
    const Rectf viewerRange(-379, 379, -279, 279); // slightly smaller than windowViewRange
    const Point2f &windowSize = iRenderQueue::getSingleton().getWindowSize();
    const Rectf windowRange(0, windowSize.x, 0, windowSize.y);
    const Rectf windowViewRange(windowRange.leftTop.x + 20, windowRange.rightBottom.x - 20, windowRange.leftTop.y + 20, windowRange.rightBottom.y - 20);
    const Point2f posOffset(windowRange.rightBottom / 2);
    const Point2f viewerPos = iRenderQueue::getSingleton().getViewerPos() + posOffset;
    Point2f pos;

    deltaTime;

    for (vector<iAttacheeEntity*>::const_iterator ie = getEntity().entities.begin(); ie != getEntity().entities.end(); ++ie)
    {
        if (((iTargetEntity*)*ie)->getEntity().getStatus(ESI_Position, &pos))
        {
            Point2f dir = pos - viewerPos;
            if (viewerRange & dir)
                continue;

            if (dir.Length() > 500)
                continue;

            float direction = -atan2(dir.y, dir.x);

            if (fabs(dir.x) > 1e-5)
            {
                Point2f pos = dir * (viewerRange.rightBottom.x / fabs(dir.x)) + posOffset;
                if (windowViewRange & pos)
                    iRenderQueue::getSingleton().render(pos, direction, frameTime, anim, GLI_UI, true);
                else
                {
                    pos = dir * (viewerRange.rightBottom.y / fabs(dir.y)) + posOffset;
                    //if (windowViewRange & pos)
                        iRenderQueue::getSingleton().render(pos, direction, frameTime, anim, GLI_UI, true);
                }
            }
            else
            {
                assert(fabs(dir.y) > 1e-5);
                Point2f pos = dir * (viewerRange.rightBottom.y / fabs(dir.y)) + posOffset;
                if (windowViewRange & pos)
                    iRenderQueue::getSingleton().render(pos, direction, frameTime, anim, GLI_UI, true);
                else
                {
                    pos = dir * (viewerRange.rightBottom.x / fabs(dir.x)) + posOffset;
                    //if (windowViewRange & pos)
                        iRenderQueue::getSingleton().render(pos, direction, frameTime, anim, GLI_UI, true);
                }
            }
        }
    }
}