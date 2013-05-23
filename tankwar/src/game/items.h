#ifndef GAME_ITEMS_H
#define GAME_ITEMS_H

#include "curvedani.h"
#include "item.h"

class ItemHP : public Item
{
public:
    ItemHP(cAni::iAnimResManager &arm) : Item(arm)
    {
        graphicEntity.init("data/item/hp.xml");
    }
    virtual void pickedBy(Tank &tank)
    {
        active = false;
        tank.fHealth += 50;
        if (tank.fHealth > 100)
            tank.fHealth = 100;
    }
};

class ItemArmor : public Item
{
public:
    ItemArmor(cAni::iAnimResManager &arm) : Item(arm)
    {
        graphicEntity.init("data/item/armor.xml");
    }
    virtual void pickedBy(Tank &tank)
    {
        active = false;
        tank.fArmor += 50;
        if (tank.fArmor > 100)
            tank.fArmor = 100;
    }
};

class ItemC1 : public Item
{
public:
    ItemC1(cAni::iAnimResManager &arm) : Item(arm)
    {
        graphicEntity.init("data/item/c1.xml");
    }
    virtual void pickedBy(Tank &tank);
};

class ItemC2 : public Item
{
public:
    ItemC2(cAni::iAnimResManager &arm) : Item(arm)
    {
        graphicEntity.init("data/item/c2.xml");
    }
    virtual void pickedBy(Tank &tank);
};

class ItemC3 : public Item
{
public:
    ItemC3(cAni::iAnimResManager &arm) : Item(arm)
    {
        graphicEntity.init("data/item/c3.xml");
    }
    virtual void pickedBy(Tank &tank);
};

class ItemC4 : public Item
{
public:
    ItemC4(cAni::iAnimResManager &arm) : Item(arm)
    {
        graphicEntity.init("data/item/c4.xml");
    }
    virtual void pickedBy(Tank &tank);
};
class ItemG1 : public Item
{
public:
    ItemG1(cAni::iAnimResManager &arm) : Item(arm)
    {
        graphicEntity.init("data/item/g1.xml");
    }
    virtual void pickedBy(Tank &tank);
};

class ItemG2 : public Item
{
public:
    ItemG2(cAni::iAnimResManager &arm) : Item(arm)
    {
        graphicEntity.init("data/item/g2.xml");
    }
    virtual void pickedBy(Tank &tank);
};

#endif//GAME_ITEMS_H