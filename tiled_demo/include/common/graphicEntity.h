#ifndef COMMON_GRAPHIC_ENTITY_H
#define COMMON_GRAPHIC_ENTITY_H

#include <vector>
#include <string>
#include "hgeCurvedani.h"
#include "caPoint2d.h"
#include "caRect.h"
#include "common/utils.h"
#include "common/entity.h"

using std::vector;
using std::string;
using cAni::iAnimation;
using cAni::Point2f;
using cAni::Rectf;
class hgeFont;


class iGraphicEntity : public Entity::iEntity
{
public:
    iGraphicEntity(Entity &e, cAni::iAnimResManager &arm) : Entity::iEntity(e), animResManager(arm)
    {
    }
    virtual void render(float gameTime, float deltaTime)
    {
        gameTime, deltaTime;
    }
    cAni::iAnimResManager &animResManager;
};

class iViewerEntity : public Entity::iEntity
{
public:
    iViewerEntity(Entity &e) : Entity::iEntity(e)
    {

    }
    virtual Point2f getViewerPos() const
    {
        return Point2f();
    }
    virtual float getViewerOrientation() const
    {
        return 0;
    }
};

#endif//COMMON_GRAPHIC_ENTITY_H