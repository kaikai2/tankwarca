#include "item.h"

Item::Item(cAni::iAnimResManager &arm) : 
    graphicEntity(*this, arm),
    collisionEntity(*this)
{

}

Item::GraphicEntity::GraphicEntity(Item &item, cAni::iAnimResManager &arm) : iGraphicEntity(item, arm)
{
    anim = iSystem::GetInstance()->createAnimation(1);
    //anim->setAnimData(arm.getAnimData("data/tank/smoke.xml"), 0);
    //anim->setAnimData(arm.getAnimData("data/tank/bulletexplode.xml"), 1);
}

Item::GraphicEntity::~GraphicEntity()
{
    iSystem::GetInstance()->release(this->anim);
    this->anim = 0;
}

void Item::GraphicEntity::init(const char *xml)
{
    assert(anim && xml);
    const cAni::AnimData *pData = animResManager.getAnimData(xml);
    anim->setAnimData(pData, 0);
    anim->startAnim(rand() % 200);
}

void Item::GraphicEntity::render(float gameTime, float deltaTime)
{
    deltaTime;

    iRenderQueue::getSingleton().render(getEntity().pos, 0, int(gameTime * 60), anim, GLI_Item);
}
