#include "tiledMap.h"
#include <algorithm>
using std::remove;


TileNull TiledMap::nullTile;

void MapObject::OnRegister(TiledMap &rTiledMap, iTile &rTile)
{
    rTile;
    for (size_t i = 0; i < occupyedPos.size(); i++)
    {
        iTile &rOccupiedTile = rTiledMap.GetTile(this->curPos + occupyedPos[i]);
        rOccupiedTile.SetOccupy(*this);
    }
}

void MapObject::OnUnregister(TiledMap &rTiledMap, iTile &rTile)
{
    rTile;
    for (size_t i = 0; i < occupyedPos.size(); i++)
    {
        iTile &rOccupiedTile = rTiledMap.GetTile(this->curPos + occupyedPos[i]);
        rOccupiedTile.ReleaseOccupy(*this);
    }
}

void MapObject::Update()
{
    lastPos = curPos;
}

void Tile::Register(TiledMap &rTiledMap, MapObject &rObj)
{
    objects.push_back(&rObj);
    rObj.OnRegister(rTiledMap, *this);
}

void Tile::Unregister(TiledMap &rTiledMap, MapObject &rObj)
{
    rObj.OnUnregister(rTiledMap, *this);
    objects.erase(remove(objects.begin(), objects.end(), &rObj), objects.end());
}

void Tile::SetOccupy(MapObject &rObj)
{
    occupiedObjs.push_back(&rObj);
}

void Tile::ReleaseOccupy(MapObject &rObj)
{
    occupiedObjs.erase(remove(occupiedObjs.begin(), occupiedObjs.end(), &rObj), occupiedObjs.end());
}

void TiledMap::OnMove(MapObject &rObj)
{
    GetTile(rObj.GetLastPos()).Unregister(*this, rObj);
    GetTile(rObj.GetCurPos()).Register(*this, rObj);
}

iTile &TiledMap::GetTile(const TilePos &rTilePos)
{
    size_t index = _FindIndex(rTilePos);
    if (index < tiles.size())
    {
        return *tiles[index];
    }
    return nullTile;
}

size_t TiledMap::_FindIndex(const TilePos &rTilePos)
{
    if (rTilePos.x < borderMin.x || rTilePos.x >= borderMax.x ||
        rTilePos.y < borderMin.y || rTilePos.x >= borderMax.y)
        return size_t(-1);
    return (rTilePos.x - borderMin.x) + (rTilePos.y - borderMin.y) * (borderMax.x - borderMin.x);
}

void MapObjectManager::UpdateObjects()
{
    for (size_t i = 0; i < objects.size(); i++)
    {
        MapObject *pMapObj = objects[i];
        pMapObj->Update();
        if (pMapObj->GetCurPos() != pMapObj->GetLastPos())
        {
            TiledMap tm; // get the tile map from where ?
            tm.OnMove(*pMapObj);
        }
    }
}
