#include <cassert>
#include <algorithm>

#include <hge.h>

#include "common/graphicEntity.h"

#include "weapon.h"
#include "gameDef.h"

using std::min;
#ifndef NULL
#define NULL 0
#endif

static ExplosiveInfo explosiveInfo_list[] =
{
    // fDamage, nPenetrability, fDamageRange
    ExplosiveInfo(200.f, 1, 50.f),
    ExplosiveInfo(160.f, 1, 50.f),
    ExplosiveInfo(100.f, 1, 50.f),
    ExplosiveInfo(300.f, 1, 50.f),
};

static Cannon cannon_list[] = 
{
    // name, nPrice, | nGrade, fShootRange, fReloadDelay, fDefinition, fDamage, fMuzzleVelocity, nPenetrability, explosive
    Cannon("AA Cannon",   100, 1,   350.f, 1.f, 0.001f,  60.f, 600.0f, 30, explosiveInfo_list + 0),
    Cannon("AA Cannon 2", 200, 2,   400.f, 1.f, 0.001f,  65.f, 650.0f, 32, explosiveInfo_list + 1),
    Cannon("AP Cannon",   300, 2,   350.f, 3.f, 0.0008f, 75.f, 750.0f, 37, explosiveInfo_list + 2),
    Cannon("AX Cannon",   500, 2,   320.f, 3.f, 0.0005f, 55.f, 550.0f, 27, explosiveInfo_list + 3),
};

Cannon* Cannon::GetData(int index)
{
    assert(index >= 0);
    if (index < sizeof(cannon_list) / sizeof(cannon_list[0]))
    {
        return cannon_list + index;
    }
    return NULL;
}

static Gun gun_list[] = 
{
    // name, nPrice, | nGrade, fShootRange, fReloadDelay, fDefinition, fDamage, fMuzzleVelocity, nPenetrability,
    Gun("AK-47 modified", 20, 1, 280.f, 0.123f, 0.02f, 10.f, 800.0f, 40),
    Gun("Scatter",        50, 2, 250.f, 0.031f, 0.05f,  8.f, 700.0f, 35),
};

Gun* Gun::GetData(int index)
{
    assert(index >= 0);
    if (index < sizeof(gun_list) / sizeof(gun_list[0]))
    {
        return gun_list + index;
    }
    return NULL;
}

static Barbette barbette_list[] = 
{
    // name, nPrice, | cannonGradeCapacity, gunGradeCapacity, fSightRange, fDirAngleRange, fMoveSpeed, nArmor
    Barbette("Eagle", 500, 1, 0, 200.f, 96.f, 50.f, 50),
    Barbette("Monster", 800, 2, 0, 150.f, 90.f, 50.f, 80),
    Barbette("Shooter", 700, 1, 0, 140.f, 152.f, 60.f, 50),
    Barbette("Apple", 1000, 2, 0, 180.f, 152.f, 50.f, 100),

    Barbette("Gun", 1000, 0, 2, 180.f, 152.f, 200.f, 100),
};

Barbette* Barbette::GetData(int index)
{
    assert(index >= 0);
    if (index < sizeof(barbette_list) / sizeof(barbette_list[0]))
    {
        return barbette_list + index;
    }
    return NULL;
}

void Weapon::WeaponInstance::render(float fGameTime, float fDeltaTime, const Point2f &pos, float attackLen, float orientation)
{
    fGameTime, fDeltaTime;

	const float fShootRange = weapon.fShootRange;
    float alpha = 1 - fabs(attackLen - fShootRange) / fShootRange;
  
	Point2f dirX(cos(orientation), sin(orientation));
    Point2f dirY(-dirX.y, dirX.x);
    Point2f from;
    Point2f to;
    from = dirX * max(0, attackLen - 10) + dirY * 0;
    to = dirX * (attackLen + 10) + dirY * 0;
    iRenderQueue::getSingleton().render(from + pos, to + pos, ARGB(128 * alpha, 255, 255, 255), GLI_UI);

    float lastx, lasty;
    int i = -3;
    lastx = fShootRange * cosf(i * 0.05f);
    lasty = fShootRange * sinf(i * 0.05f);
    for (i++; i <= 3; i++)
    {
        float x = fShootRange * cosf(i * 0.05f);
        float y = fShootRange * sinf(i * 0.05f);
      
        from = dirX * lastx + dirY * lasty;
        to = dirX * x + dirY * y;
        iRenderQueue::getSingleton().render(from + pos, to + pos, ARGB(128 * alpha, 255, 255, 255), GLI_UI);
        lastx = x, lasty = y;
    }
}

void Barbette::BarbetteInstance::step(float fGameTime, float fDeltaTime)
{
    fGameTime;
    Point2f attackDirection = posAttackTarget - posCenter;
    if (attackDirection.DotProduct() > 1)
    {
        attackDirection.Normalize();
        
        //const float fBaseDirection = base.getDirection();
        //assert(fBaseDirection >= 0 && fBaseDirection <= 360);

        float rad = this->getDirection() / 180.f * Pi();
        Point2f dir = Point2f(cosf(rad), sinf(rad));

        assert(fBarbetteDirection >= -barbette.fDirAngleRange &&
            fBarbetteDirection <= barbette.fDirAngleRange);
        float destRad = acosf(attackDirection * dir) * 180 / Pi();//fabs(180.f * acosf(1.f - (attackDirection - dir).DotProduct() * .5f) / Pi);
        float deltaAng = min(destRad, barbette.fMoveSpeed * fDeltaTime);
       /* 
        float baseRad = fBaseDirection / 180.f * Pi;
        Point2f baseDirection = Point2f(cosf(baseRad), sinf(baseRad));
        float destOffRad = 180.f * acosf(attackDirection * baseDirection) / Pi;
        if (attackDirection ^ baseDirection < 0)
        destOffRad = -destOffRad;
        if (destOffRad > barbette.fDirAngleRange)
        destOffRad = barbette.fDirAngleRange;
        else if (destOffRad < -barbette.fDirAngleRange)
        destOffRad = -barbette.fDirAngleRange;
        attackDirection = Point2f(cosf(destOffRad), sinf(destOffRad));
        float deltaAng = min(destOffRad, barbette.fMoveSpeed * fDeltaTime);
        */
        if ((attackDirection ^ dir) < 0)
        {
            if (fBarbetteDirection < barbette.fDirAngleRange)
            {
                fBarbetteDirection += min(barbette.fDirAngleRange - fBarbetteDirection, deltaAng);
            }
        }
        else
        {
            if (fBarbetteDirection > -barbette.fDirAngleRange)
            {
                fBarbetteDirection -= min(fBarbetteDirection + barbette.fDirAngleRange, deltaAng);
            }
        }
    }
}

static Enginee enginee_list[] = 
{
    // name, nPrice, | fPower, fMaxAcceleration
    Enginee("ouse", 300, 500.f, 50.f),
    Enginee("House", 400, 600.f, 55.f),
    Enginee("Flea",  400, 300.f, 70.f),
};

Enginee* Enginee::GetData(int index)
{
    assert(index >= 0);
    if (index < sizeof(enginee_list) / sizeof(enginee_list[0]))
    {
        return enginee_list + index;
    }
    return NULL;
}
