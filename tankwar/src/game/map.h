#ifndef GAME_MAP_H
#define GAME_MAP_H

#include <cassert>
#include <vector>
#include <list>

#include "common/attachEntity.h"

#include "collisionDef.h"
#include "routeMap.h"

using std::vector;
using std::list;
using cAni::Point2f;

class Map : public Entity
{
public:
    enum EntityInterfaceIds
    {
        EII_QueryEntity = Entity::NumEntityInterfaceIds,

        NumEntityInterfaceIds,
    };
    Map(cAni::iAnimResManager &arm);
    virtual ~Map();
    void clear();
    /*
    void loadMap(const char *baseObjs, const char *mapfile)
    {
        
    }
    */
    void randomizeMap(const Rectf &range, const char *baseObjs, size_t wetland, size_t stone, size_t box);
    void getRandomPos(Point2f &pos) const;
    void findPath(list<Point2f> &result, const Point2f &fromPos, const Point2f &toPos) const
    {
        rmap.FindPath(result, fromPos, toPos);
    }
    virtual void step(float gameTime, float deltaTime);
    virtual iEntity* getEntityInterface(EntityInterfaceId id)
    {
        switch(id)
        {
        case EII_QueryEntity:
            return &mapQueriers;
        case Entity::EII_GraphicEntity:
            return &graphicEntity;
        }
        return 0;
    }
    class GraphicEntity : public iGraphicEntity
    {
    public:
        GraphicEntity(Entity &e, cAni::iAnimResManager &arm) : iGraphicEntity(e, arm), tex(0)
        {
        }
        virtual ~GraphicEntity()
        {
            if (tex)
            {
                HGE *hge = hgeCreate(HGE_VERSION);
                hge->Texture_Free(tex);
                tex = 0;
                hge->Release();
            }
        }
        virtual void render(float gameTime, float deltaTime);
    protected:
        Map &getEntity()
        {
            return *(Map*)&entity;
        }
        HTEXTURE tex;
    };
    const RouteMap &getRouteMap() const
    {
        return rmap;
    }
protected:
    void addMapObject(Entity &obj);
    vector<Entity*> objects;
    cAni::iAnimResManager *animResManager;

    RouteMap rmap;

    iMultiAttacheeEntity mapQueriers;
    GraphicEntity graphicEntity;
};

#endif