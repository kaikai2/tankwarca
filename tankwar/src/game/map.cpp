#pragma warning(disable: 4996)

#include <string>
#include <cassert>
#include <algorithm>

#include "map.h"
#include "mapObj.h"
#include "staticMapObj.h"
#include "common/entityManager.h"
#include "common/graphicEntity.h"

using std::vector;


Map::Map(cAni::iAnimResManager &arm) : animResManager(&arm),
    mapQueriers(*this),
    graphicEntity(*this, arm)
{
}

Map::~Map()
{
    clear();
}

void Map::clear()
{
    EntityManager::getSingleton().dettach(objects);
    for (size_t i = 0; i < objects.size(); ++i)
    {
        delete objects[i];
    }
    objects.clear();
}

void Map::step(float gameTime, float deltaTime)
{
    gameTime, deltaTime;

    for (size_t i = 0; i < objects.size(); ++i)
    {
        Entity *maoObj = objects[i];
        // TODO: check mapObj.isActive(), remove it
    }
}

void Map::GraphicEntity::render(float gameTime, float deltaTime)
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

    iRenderQueue::getSingleton().render(q, GLI_MapGround, true);

#ifdef PATH_FINDING_TOOL

#if PATH_FINDING_TOOL
#ifndef NDEBUG
    getEntity().getRouteMap().renderDebug();
#endif
#endif

#else

//#ifndef NDEBUG
    getEntity().getRouteMap().renderDebug();
//#endif

#endif
}

struct Node
{
    string xmlFileName;
    int w, h;
};

void Map::randomizeMap(const Rectf &range, const char *baseObjs, size_t wetland, size_t stone, size_t box)
{
    baseObjs;

    vector<Node> nodes;
    vector<Node> nodesStatic;
    vector<Node> nodesWetland;
    FILE *fp = fopen("data/map/mapdata.txt", "rt");
    int line;
    fscanf(fp, "%d", &line);
    for (int i = 0; i < line; i++)
    {
        char type[128];
        char s[128];
        int w, h;
        fscanf(fp, "%s%s%d%d", type, s, &w, &h);
        Node n;
        n.xmlFileName = s;
        n.w = w;
        n.h = h;
        if (string(type) == "block")
            nodes.push_back(n);
        else if (string(type) == "wetland")
            nodesWetland.push_back(n);
        else if (string(type) == "staticblock")
            nodesStatic.push_back(n);
    }
    fclose(fp);

    if (nodes.empty() && nodesStatic.empty() && nodesWetland.empty())
        return;

    int maxw = 0, maxh = 0;
    size_t maxwI = 0, maxhI = 0;
    float rotateW = 0;
    float rotateH = 0;

    rmap.setRange(range);

    Rectf safeRange(range);
    safeRange.leftTop += Point2f(100.f, 100.f);
    safeRange.rightBottom += Point2f(-100.f, -100.f);

    size_t i;
    if (!nodesStatic.empty())
    {
        for (i = 0; i < nodesStatic.size(); i++)
        {
            Node &n = nodesStatic[i];
            if (n.w > maxw)
                maxw = n.w, maxwI = i;
            if (n.h > maxh)
                maxh = n.h, maxhI = i;
        }
        if (maxw > maxh)
        {
            maxh = maxw;
            maxhI = maxwI;
            rotateH = Pi() / 2;
        }
        else if (maxw < maxh)
        {
            maxw = maxh;
            maxwI = maxhI;
            rotateW = Pi() / 2;
        }
        for (float x = safeRange.leftTop.x + maxw * 16.f; x < safeRange.rightBottom.x + maxw * 16.f + 1.f; x += maxw * 32.f - 1.f)
        {
            StaticMapObj *mo = new StaticMapObj(*animResManager);
            Node &n = nodesStatic[maxwI];
            EntityManager::getSingleton().attach(mo);
            mo->init(n.xmlFileName.c_str(), n.w * 32.0f, n.h * 32.0f, rotateW);
            mo->setPosition(cAni::Point2f(x, safeRange.leftTop.y));
            addMapObject(*mo);

            mo = new StaticMapObj(*animResManager);
            EntityManager::getSingleton().attach(mo);
            mo->init(n.xmlFileName.c_str(), n.w * 32.0f, n.h * 32.0f, rotateW);
            mo->setPosition(cAni::Point2f(x, safeRange.rightBottom.y));
            addMapObject(*mo);
        }

        for (float y = safeRange.leftTop.y + maxh * 16.f; y < safeRange.rightBottom.y + maxh * 16.f + 1.f; y += maxh * 32.f - 1.f)
        {
            StaticMapObj *mo = new StaticMapObj(*animResManager);
            Node &n = nodesStatic[maxhI];
            EntityManager::getSingleton().attach(mo);
            mo->init(n.xmlFileName.c_str(), n.w * 32.0f, n.h * 32.0f, rotateH);
            mo->setPosition(cAni::Point2f(safeRange.leftTop.x, y));
            addMapObject(*mo);

            mo = new StaticMapObj(*animResManager);
            EntityManager::getSingleton().attach(mo);
            mo->init(n.xmlFileName.c_str(), n.w * 32.0f, n.h * 32.0f, rotateH);
            mo->setPosition(cAni::Point2f(safeRange.rightBottom.x, y));
            addMapObject(*mo);
        }
    }
    for (i = 0; i < wetland; i++)
    {
        Wetland *mo = new Wetland(*animResManager);
        Node &n = nodesWetland[rand() % nodesWetland.size()];
        EntityManager::getSingleton().attach(mo);
        mo->init(n.xmlFileName.c_str(), n.w * 32.0f, n.h * 32.0f, (rand() % 360 / 360.0f) * Pi());
        mo->setPosition(cAni::Point2f(rand() % (int)safeRange.GetWidth() + safeRange.leftTop.x, rand() % (int)safeRange.GetHeight() + safeRange.leftTop.y));
        addMapObject(*mo);
    }
    for (i = 0; i < stone; i++)
    {
        StaticMapObj *mo = new StaticMapObj(*animResManager);
        Node &n = nodesStatic[rand() % nodesStatic.size()];
        EntityManager::getSingleton().attach(mo);
        mo->init(n.xmlFileName.c_str(), n.w * 32.0f, n.h * 32.0f, (rand() % 360 / 360.0f) * Pi());
        mo->setPosition(cAni::Point2f(rand() % (int)safeRange.GetWidth() + safeRange.leftTop.x, rand() % (int)safeRange.GetHeight() + safeRange.leftTop.y));
        addMapObject(*mo);
    }
    for (i = 0; i < box; i++)
    {
        MapObj *mo = new MapObj(*animResManager);
        Node &n = nodes[rand() % nodes.size()];
        EntityManager::getSingleton().attach(mo);
        mo->init(n.xmlFileName.c_str(), n.w * 16.0f, n.h * 16.0f);
        mo->setPosition(cAni::Point2f(rand() % (int)safeRange.GetWidth() + safeRange.leftTop.x, rand() % (int)safeRange.GetHeight() + safeRange.leftTop.y));
        addMapObject(*mo);
    }
    //rmap.randomVertices(range);
    rmap.generateMapFromVertices();
}

void Map::addMapObject(Entity &obj)
{
    objects.push_back(&obj);
    switch(obj.getTypeId())
    {
    case ETI_Box:
    case ETI_Wetland:
        {
            StaticMapObj &mapobj = static_cast<StaticMapObj &>(obj);
            vector<Point2f> v;
            mapobj.getBoundingVertices(v);
            // rmap.verticesBuffer.insert(rmap.verticesBuffer.end(), v.begin(), v.end());
            rmap.addPolygon(v);
        }
        break;
    default:
        break;
    }
}

void Map::getRandomPos(Point2f &pos) const
{
    if (rmap.triangles.empty())
    {
        return;
    }

    size_t nIndex = rand() % rmap.triangles.size();
    const RouteMap::Triangle &t = rmap.triangles[nIndex];
    Point2f p1 = rmap.verticesBuffer[t.v[0]];
    Point2f p2 = rmap.verticesBuffer[t.v[1]];
    Point2f p3 = rmap.verticesBuffer[t.v[2]];

    float f1 = (float)(rand() % RAND_MAX) / (float)RAND_MAX;
    float f2 = (float)(rand() % RAND_MAX) / (float)RAND_MAX;

    if (f2 + f1 > 1.f)
    {
        f1 = 1.f - f1;
        f2 = 1.f - f2;
    }

    pos = (p2 - p1) * f1 + (p3 - p1) * f2 + p1;
}
