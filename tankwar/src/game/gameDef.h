#ifndef GAME_GAME_DEF_H
#define GAME_GAME_DEF_H

#include "common/iRenderQueue.h"

enum EntityTypeIds
{
    ETI_Tank = 1,
    ETI_Bullet,
    ETI_CannonBall,
    ETI_Item,
    ETI_Explosive,
    ETI_Box,
    ETI_Wetland,
};

enum GraphicsLayerIds
{
    GLI_UI = iRenderQueue::LayerDebugInfo + 1,

    GLI_Item,
    GLI_Bullets,
    GLI_Tank,

    GLI_MapObj1,
    GLI_MapObj0,
    GLI_MapGround,

    NumGraphicsLayerId,
};

#endif//GAME_GAME_DEF_H