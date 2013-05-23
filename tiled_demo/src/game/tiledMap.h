#ifndef GAME_TILED_MAP_H
#define GAME_TILED_MAP_H

#include <cassert>
#include <vector>
#include "ca/caPoint2d.h"

using std::vector;

struct TilePos
{
    int x;
    int y;
    TilePos() : x(0), y(0)
    {
    }
    TilePos(int _x, int _y) : x(_x), y(_y)
    {
    }
    TilePos operator + (const TilePos &o) const
    {
        return TilePos(x + o.x, y + o.y);
    }
    TilePos operator - (const TilePos &o) const
    {
        return TilePos(x - o.x, y - o.y);
    }
	bool operator == (const TilePos &o) const
	{
		return x == o.x && y == o.y;
	}
	bool operator != (const TilePos &o) const
	{
		return !(*this == o);
	}
};
typedef TilePos TileOffPos;

class TiledMap;
class MapObject;

class iTile
{
public:
    virtual void Register(TiledMap &rTiledMap, MapObject &rObj) = 0;
    virtual void Unregister(TiledMap &rTiledMap, MapObject &rObj) = 0;
    virtual void SetOccupy(MapObject &rObj) = 0;
    virtual void ReleaseOccupy(MapObject &rObj) = 0;
    virtual const TilePos &GetTilePos() const = 0;
};

class MapObject
{
public:
    void OnRegister(TiledMap &rTiledMap, iTile &rTile);
    void OnUnregister(TiledMap &rTiledMap, iTile &rTile);
    void Update();
    const TilePos &GetCurPos() const{return curPos;}
    const TilePos &GetLastPos() const{return lastPos;}
protected:
    vector<TileOffPos> occupyedPos;
    TilePos curPos;
    TilePos lastPos;
};

class MapObjectManager
{
public:
    void UpdateObjects();
protected:
    vector<MapObject*> objects;
};

class Tile : public iTile
{
public:
    Tile(TiledMap &_map) : map(_map)
    {
    }
    virtual void Register(TiledMap &rTiledMap, MapObject &rObj);
    virtual void Unregister(TiledMap &rTiledMap, MapObject &rObj);
    virtual void SetOccupy(MapObject &rObj);
    virtual void ReleaseOccupy(MapObject &rObj);
    virtual const TilePos &GetTilePos() const
    {
        return pos;
    }
    void SetTilePos(const TilePos &_pos)
    {
        pos = _pos;
    }

    cAni::Point2f getPosition();
protected:
    Tile &operator = (const Tile &rhs)
    {
        assert(0);
        rhs;
        return *this;
    }
    TiledMap &map;
    TilePos pos;
    vector<MapObject*> objects;
    vector<MapObject*> occupiedObjs;
};

class TileNull : public iTile
{
public:
    virtual void Register(TiledMap &, MapObject &)
    {
    }
    virtual void Unregister(TiledMap &, MapObject &)
    {
    }
    virtual void SetOccupy(MapObject &)
    {
    }
    virtual void ReleaseOccupy(MapObject &)
    {
    }
    virtual const TilePos &GetTilePos() const
    {
        static TilePos nullPos;
        return nullPos;
    }
};

class TiledMap
{
public:
    TiledMap()
    {
        borderMin = TilePos(-10, -10);
        borderMax = TilePos(10, 10);
    }
    void OnMove(MapObject &rObj);
    iTile &GetTile(const TilePos &rTilePos);

    void AddTile(iTile &tile)
    {
        size_t id = _FindIndex(tile.GetTilePos());
        if (id >= tiles.size())
        {
            size_t size = tiles.size();
            tiles.resize(id + 1);
            for (size_t i = size; i < tiles.size(); i++)
            {
                tiles[i] = 0;
            }
        }
        tiles[id] = &tile;
    }
    virtual void ClearTile()
    {
        tiles.clear();
    }
    cAni::Point2f mapTilePosToPosition(const TilePos &rTilePos)
    {
        TilePos borderSize = borderMax - borderMin;
        return cAni::Point2f(
            centerPos.x + tileSize.x * ((rTilePos.x - rTilePos.y + borderSize.x) / 2.f - 10),
            centerPos.y + tileSize.y * ((rTilePos.x + rTilePos.y) / 2.f - borderSize.y + 20));
    }
    cAni::Point2f centerPos;
    cAni::Point2f tileSize;
protected:
    size_t _FindIndex(const TilePos &rTilePos);

    vector<iTile*> tiles;
    TilePos borderMin, borderMax;

    static TileNull nullTile;
};

inline
cAni::Point2f Tile::getPosition()
{
    return map.mapTilePosToPosition(pos);
}

#endif//GAME_TILED_MAP_H
