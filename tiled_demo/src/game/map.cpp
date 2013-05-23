#pragma warning(disable: 4996)

#include <string>
#include <cassert>
#include <algorithm>

#include "map.h"
#include "common/entityManager.h"
#include "common/graphicEntity.h"
#include "common/iRenderQueue.h"

using std::vector;


TiledMapEntity::TiledMapEntity(cAni::iAnimResManager &/*arm*/)// :
    //animResManager(&arm),
    //mapQueriers(*this),
    //graphicEntity(*this, arm)
{
}

TiledMapEntity::~TiledMapEntity()
{
}

void TiledMapEntity::step(float gameTime, float deltaTime)
{
    gameTime, deltaTime;

    //for (size_t i = 0; i < objects.size(); ++i)
    //{
    //    Entity *maoObj = objects[i];
        // TODO: check mapObj.isActive(), remove it
    //}
}
/*
void TiledMapEntity::GraphicEntity::render(float gameTime, float deltaTime)
{
    gameTime, deltaTime;

    Point2f viewerPos = iRenderQueue::getSingleton().getViewerPos();
    const Point2f &windowSize = iRenderQueue::getSingleton().getWindowSize();
    hgeQuad q;
    q.v[0].x = 0;
    q.v[1].x = 0;
    q.v[3].x = q.v[2].x = windowSize.x;

    q.v[0].y = 0;
    q.v[1].y = q.v[2].y = windowSize.y;
    q.v[3].y = 0;

    q.v[0].z = 0;
    q.v[1].z = 0;
    q.v[2].z = 0;
    q.v[3].z = 0;

    q.v[0].col = ARGB(255, 255, 255, 255);
    q.v[1].col = ARGB(255, 255, 255, 255);
    q.v[2].col = ARGB(255, 255, 255, 255);
    q.v[3].col = ARGB(255, 255, 255, 255);

    q.v[0].tx = viewerPos.x / 256;
    q.v[1].tx = viewerPos.x / 256;
    q.v[2].tx = (viewerPos.x + windowSize.x) / 256;
    q.v[3].tx = (viewerPos.x + windowSize.x) / 256;

    q.v[0].ty = viewerPos.y / 256;
    q.v[1].ty = (viewerPos.y + windowSize.y) / 256;
    q.v[2].ty = (viewerPos.y + windowSize.y) / 256;
    q.v[3].ty = viewerPos.y / 256;
    if (!tex)
    {
        HGE *hge = hgeCreate(HGE_VERSION);
        tex = hge->Texture_Load("data/map/road_26.tga");
        hge->Release();
    }
    q.tex = tex;
    q.blend = BLEND_DEFAULT;
    // hge->Gfx_RenderQuad(&q);

    iRenderQueue::getSingleton().render(q, RenderQueue::LayerDebugInfo, true);

#ifdef PATH_FINDING_TOOL

#if PATH_FINDING_TOOL
#ifndef NDEBUG
    getEntity().getRouteMap().renderDebug();
#endif
#endif

#else

#ifndef NDEBUG
//    getEntity().getRouteMap().renderDebug();
#endif

#endif
}

*/