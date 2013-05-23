#include "items.h"
#include "weapon.h"

void ItemC1::pickedBy(Tank &tank)
{
    active = false;
    tank.SetCannon(*Cannon::GetData(0));
}

void ItemC2::pickedBy(Tank &tank)
{
    active = false;
    tank.SetCannon(*Cannon::GetData(1));
}

void ItemC3::pickedBy(Tank &tank)
{
    active = false;
    tank.SetCannon(*Cannon::GetData(2));
}

void ItemC4::pickedBy(Tank &tank)
{
    active = false;
    tank.SetCannon(*Cannon::GetData(3));
}

void ItemG1::pickedBy(Tank &tank)
{
    active = false;
    tank.SetGun(*Gun::GetData(0));
}

void ItemG2::pickedBy(Tank &tank)
{
    active = false;
    tank.SetGun(*Gun::GetData(1));
}