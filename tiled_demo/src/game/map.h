#ifndef GAME_MAP_H
#define GAME_MAP_H

#include <cassert>
#include <vector>
#include <list>

#include "common/graphicEntity.h"
#include "common/attachEntity.h"
#include "common/entityManager.h"
#include "ca/curvedani.h"
#include "common/iRenderQueue.h"
#include "tiledMap.h"

using std::vector;
using std::list;
using cAni::Point2f;

class TileEntity : public Entity, public Tile
{
public:
    TileEntity(cAni::iAnimResManager &arm, TiledMap &_map, const char *aniPath) : Tile(_map), graphicEntity(*this, arm, aniPath)
    {
    }
    virtual ~TileEntity()
    {
    }
    virtual void step(float gameTime, float deltaTime)
    {
        gameTime, deltaTime;
    }
    virtual iEntity* getEntityInterface(EntityInterfaceId id)
    {
        switch(id)
        {
        case Entity::EII_GraphicEntity:
            return &graphicEntity;
        }
        return 0;
    }
    class GraphicEntity : public iGraphicEntity
    {
    public:
        GraphicEntity(Entity &e, cAni::iAnimResManager &arm, const char *aniPath) : iGraphicEntity(e, arm)
        {
			ani = iSystem::GetInstance()->createAnimation();
			ani->setAnimData(arm.getAnimData(aniPath), 0);
			startTime = rand() % 1000;
			scale = (rand() % 500 + 500) / 1000.0f;
        }
        virtual ~GraphicEntity()
        {
			iSystem::GetInstance()->release(ani);
        }
        virtual void render(float gameTime, float deltaTime)
		{
            gameTime, deltaTime;
			if (ani)
			{
				int frame = gameTime * 60 * scale - startTime;
                iRenderQueue::getSingleton().render(getEntity().getPosition(), 0, frame, ani);
			}
		}
		int id;
		int startTime;
		float scale;
    protected:
        TileEntity &getEntity()
        {
            return *(TileEntity*)&entity;
        }
		cAni::iAnimation *ani;
    };
protected:
    cAni::iAnimResManager *animResManager;
    GraphicEntity graphicEntity;
};

class TiledMapEntity : public Entity, public TiledMap
{
public:
    enum EntityInterfaceIds
    {
    //    EII_QueryEntity = Entity::NumEntityInterfaceIds,

        NumEntityInterfaceIds,
    };
    TiledMapEntity(cAni::iAnimResManager &arm);
    virtual ~TiledMapEntity();
    void clear();

    virtual void step(float gameTime, float deltaTime);
    virtual iEntity* getEntityInterface(EntityInterfaceId/*id*/)
    {
        //switch(id)
        {
        //case EII_QueryEntity:
        //    return &mapQueriers;
        //case Entity::EII_GraphicEntity:
        //    return &graphicEntity;
        }
        return 0;
    }
    /*
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
        TiledMap &getEntity()
        {
            return *(TiledMap*)&entity;
        }
        HTEXTURE tex;
    };
    */

    void AddTile(TileEntity &tile)
    {
        TiledMap::AddTile(tile);

        EntityManager::getSingleton().attach(&tile);
    }

    virtual void ClearTile()
    {
        TiledMap::ClearTile();

        for (size_t i = 0; i < tiles.size(); i++)
        {
            EntityManager::getSingleton().dettach((TiledMapEntity *)tiles[i]);
        }
    }

   /* TileEntity &GetTile(size_t i)
    {
        if (i < tiles.size())
        {
            return *tiles[i];
        }
        return 
    }
*/
protected:
    cAni::iAnimResManager *animResManager;
    //iMultiAttacheeEntity mapQueriers;
    //GraphicEntity graphicEntity;
};

#endif