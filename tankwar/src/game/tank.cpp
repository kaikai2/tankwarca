#pragma warning(disable: 4996)

#include <cstdio>
#include <hge.h>

#include "tank.h"
#include "weapon.h"
#include "cannonBall.h"
#include "bullet.h"
#include "explosive.h"
#include "cooldownIcon.h"
#include "common/graphicEntity.h"
#include "common/entityManager.h"
#include "common/soundManager.h"
#include "common/iRenderQueue.h"

#define Pi Pi()

TransitionMap s_tmMove;

struct GetReadyForTankMoveTransitonMap 
{
	GetReadyForTankMoveTransitonMap()
	{
		s_tmMove.AddRule("Stay", 1, "Moving");
		s_tmMove.AddRule("Moving", 2, "Stay");
	}
};

Tank::Tank(cAni::iAnimResManager &arm, hgeFont *font) :
    Entity(),
    fDirection(0.f), fSpeed(100.f), mMoveStatus(0), mAttackStatus(0), fGunDirection(0.f),
    mBarbette(NULL), mCannon(NULL), mGunBarbette(NULL), mGun(NULL),
    collisionEntity(*this),
    controlEntity(*this),
    graphicEntity(*this, arm, font),
    viewerEntity(*this),
    radarEntity(*this),
    targetEntity(*this),
    gunAttacheeEntity(*this),
    cannonAttacheeEntity(*this),
    bShowAim(false),
    bAmphibious(false),
    state(TS_Alive),
	fsmMove(s_tmMove)
{
    fHealth = 100;
    fArmor = 100;
    turnDirection = 0;
    fCurSpeed = 0;
	fsmMove.Start("Stay");
}

Tank::~Tank()
{
    if (mCannon)
        delete mCannon;
    if (mGun)
        delete mGun;
    if (mBarbette)
        delete mBarbette;
    if (mGunBarbette)
        delete mGunBarbette;
}

void Tank::Fire(const Weapon::WeaponCondition &rWeaponCondition) // 主炮 开火
{
    if (mCannon == NULL || !mCannon->isReadyToFire(rWeaponCondition))
        return;

    mCannon->fire(rWeaponCondition);
    
    SoundManager::getSingleton().play("data/se/tk_expb.WAV", pos);

    CannonBall *pBall = new CannonBall(this->graphicEntity.animResManager);
    EntityManager::getSingleton().attach(pBall);
    assert(pBall != NULL);

    ((iCollisionEntity*)pBall->getEntityInterface(EII_CollisionEntity))->addIgnoredEntities(*(iCollisionEntity*)this->getEntityInterface(EII_CollisionEntity));
    float rad = (mBarbette ? mBarbette->getDirection() : this->getDirection()) / 180.f * Pi;
    float off = (((float)rand() / RAND_MAX) - 0.5f) * atan(mCannon->getWeapon().fDefinition);
    rad += off;
    pBall->orientation = rad;
    Point2f dir0 = Point2f(cosf(rad), sinf(rad));
    float fShootRange = mCannon->getWeapon().fShootRange;
    if (mBarbette)
    {
        float attackDistance = (mBarbette->getAttackPos() - pos).Length();
        if (attackDistance <= fShootRange)
            fShootRange = attackDistance;
    }
    Point2f dir = dir0 * fShootRange;
    pBall->setPostion(pos/* + dir0 * getSize() * 1.5*/, pos + dir);
    pBall->cannon = (const Cannon *)&mCannon->getWeapon();
    pBall->speed = mCannon->getWeapon().fMuzzleVelocity;
}

void Tank::Shoot(const Weapon::WeaponCondition &rWeaponCondition)// 机枪 开火
{
    if (mGun == NULL || !mGun->isReadyToFire(rWeaponCondition))
        return;

    mGun->fire(rWeaponCondition);
    SoundManager::getSingleton().play("data/se/tk_fire.WAV", pos, 0.2f);

    Bullet *pBall = new Bullet(this->graphicEntity.animResManager);
    EntityManager::getSingleton().attach(pBall);
    assert(pBall != NULL);

    ((iCollisionEntity*)pBall->getEntityInterface(EII_CollisionEntity))->addIgnoredEntities(*(iCollisionEntity*)this->getEntityInterface(EII_CollisionEntity));
    float rad = (mGunBarbette ? mGunBarbette->getDirection() : this->getDirection()) / 180.f * Pi;
    float off = (((float)rand() / RAND_MAX) - 0.5f) * atan(mGun->getWeapon().fDefinition);
    rad += off;
    pBall->orientation = rad;
    Point2f dir0 = Point2f(cosf(rad), sinf(rad));

    float fShootRange = mGun->getWeapon().fShootRange;
    if (mGunBarbette)
    {
        float attackDistance = (mGunBarbette->getAttackPos() - pos).Length();
        if (attackDistance <= fShootRange)
            fShootRange = attackDistance;
    }
    Point2f dir = dir0 * fShootRange;
    pBall->setPostion(pos/* + dir0 * getSize() * 1.5*/, pos + dir);
    pBall->gun = (const Gun *)&mGun->getWeapon();
    pBall->speed = mCannon->getWeapon().fMuzzleVelocity;
}

void Tank::step(float gameTime, float deltaTime)
{
    switch(state)
    {
    case TS_Alive:
        {
            assert(fDirection >= 0 && fDirection <= 360);
            float rad = fDirection / 180.f * Pi;
            float speed = fSpeed;
            if (this->mMoveStatus & MS_HalfEnginePower)
                speed *= 0.5f;

            // speed up and down
            const float fEpsilon = 100.f * deltaTime;
            const float fHalfEpsilon = fEpsilon * 0.5f;
            if (mMoveStatus & (MS_UP|MS_DOWN))
            {
                if (fCurSpeed < speed - fHalfEpsilon)
                {
                    fCurSpeed += fEpsilon;
                    if (fCurSpeed > speed - fEpsilon)
                    {
                        fCurSpeed = speed;
                    }
                }
                else if (fCurSpeed > speed + fHalfEpsilon)
                {
                    fCurSpeed -= fEpsilon;
                    if (fCurSpeed < speed + fEpsilon)
                    {
                        fCurSpeed = speed;
                    }
                }
            }
            else
            {
                fCurSpeed -= fEpsilon;
                if (fCurSpeed < fEpsilon)
                    fCurSpeed = 0;
            }

            const Point2f dir = Point2f(cosf(rad), sinf(rad)) * (deltaTime);
            Point2f newPos = pos;
            if ((mMoveStatus & (MS_UP|MS_DOWN)) == MS_UP)
            {
                newPos += dir * fCurSpeed;
            }
            else if ((mMoveStatus & (MS_UP|MS_DOWN)) == MS_DOWN)
            {
                newPos -= dir * fCurSpeed;
            }
            if (setTurnDirection)
            {
                float diffDir = fabs(turnDirection - fDirection);
                diffDir -= int(diffDir / 360) * 360.f;
                if (diffDir > 1.f)
                {
                    float radTD = turnDirection / 180.f * Pi;
                    Point2f dir2(cos(radTD), sin(radTD));
                    float d = dir ^ dir2;
                    if (d < 0)
                    {
                        this->AddMoveStatus(MS_LEFT);
                        this->RemoveMoveStatus(MS_RIGHT);
                    }
                    else if (d > 0)
                    {
                        this->AddMoveStatus(MS_RIGHT);
                        this->RemoveMoveStatus(MS_LEFT);
                    }
                }
                else
                {
                    setTurnDirection = false;
                    this->RemoveMoveStatus(MS_RIGHT);
                    this->RemoveMoveStatus(MS_LEFT);
                }
            }

            if ((mMoveStatus & (MS_LEFT|MS_RIGHT)) == MS_LEFT)
            {
                fDirection -= 50 * deltaTime;
                if (fDirection < 0)
                    fDirection += 360;
            }
            else if ((mMoveStatus & (MS_LEFT|MS_RIGHT)) == MS_RIGHT)
            {
                fDirection += 50 * deltaTime;
                if (fDirection >= 360)
                    fDirection -= 360;
            }

			Weapon::WeaponCondition wc;
			wc.fCurTime = gameTime;
            if (mAttackStatus & AS_Cannon)
                Fire(wc);
            if (mAttackStatus & AS_Gun)
                Shoot(wc);

            if (mBarbette)
            {
                mBarbette->step(gameTime, deltaTime);
            }
            if (mGunBarbette)
            {
                mGunBarbette->step(gameTime, deltaTime);
            }
            collisionEntity.getBody().Reset(pos, getDirection() / 180.f * Pi, 1);
            curVelocity = (newPos - pos) / deltaTime;
            collisionEntity.getBody().setVelocity(curVelocity);
            collisionEntity.getBody().Update(deltaTime);
            // pos = (newPos); // here may touch the obstacles, and not move to the exact newPos
            if (mBarbette)
                mBarbette->setCenterPos(pos);
            if (mGunBarbette)
                mGunBarbette->setCenterPos(pos);
        }
        break;
    case TS_Dead:
        if (graphicEntity.checkExplodeSmokeEnd(gameTime))
        {
            active = false;
        }
        break;
    }
}

void Tank::damage(float fDamage, int nPenetrability)
{
    if (fDamage <= 1e-2f)
        return;

    assert(fDamage >= 0 && fDamage < 1e10);
    assert(nPenetrability >= 0 && nPenetrability <= 100);
    const float actualDamage = fDamage * fDamage / (fDamage + fArmor);
    assert(actualDamage >= 0 && actualDamage < 1e10);
    float fPenetrability = 0.01f * nPenetrability;
    if (fPenetrability > 1.f)
        fPenetrability = 1.f;
    const float armorDamage = actualDamage * (1.f - fPenetrability);
    float restDamage = actualDamage - armorDamage;
    if (fArmor >= armorDamage)
        fArmor -= armorDamage;
    else
    {
        restDamage += armorDamage - fArmor;
        fArmor = 0;
    }
    fHealth -= restDamage;
    if (fHealth <= 0)
    {
        fHealth = 0;
        
        state = TS_Dead;
        graphicEntity.playSmoke();
        // this->active = false;
    }
}

void Tank::ControlEntity::onControllerCommand(int command, const void *pData)
{
    switch(command)
    {
    case TCI_TurnLeft:
        if (getTank().setTurnDirection)
        {
            getTank().RemoveMoveStatus(MS_LEFT);
            getTank().RemoveMoveStatus(MS_RIGHT);
            getTank().setTurnDirection = false;
        }

        pData ? getTank().AddMoveStatus(MS_LEFT) : getTank().RemoveMoveStatus(MS_LEFT);
        break;
    case TCI_TurnRight:
        if (getTank().setTurnDirection)
        {
            getTank().RemoveMoveStatus(MS_LEFT);
            getTank().RemoveMoveStatus(MS_RIGHT);
            getTank().setTurnDirection = false;
        }
        pData ? getTank().AddMoveStatus(MS_RIGHT) : getTank().RemoveMoveStatus(MS_RIGHT);
        break;
    case TCI_Forward:
        pData ? getTank().AddMoveStatus(MS_UP) : getTank().RemoveMoveStatus(MS_UP);
        break;
    case TCI_Backward:
        pData ? getTank().AddMoveStatus(MS_DOWN) : getTank().RemoveMoveStatus(MS_DOWN);
        break;
    case TCI_HalfEnginePower:
        pData ? getTank().AddMoveStatus(MS_HalfEnginePower) : getTank().RemoveMoveStatus(MS_HalfEnginePower);
        break;
    case TCI_Fire:
        pData ? getTank().AddAttackStatus(AS_Cannon) : getTank().RemoveAttackStatus(AS_Cannon);
        break;
    case TCI_Shoot:
        pData ? getTank().AddAttackStatus(AS_Gun) : getTank().RemoveAttackStatus(AS_Gun);
        break;
    case TCI_Aim:
        {
            assert(pData);
            const Point2f &pos = *(const Point2f *)pData;
            getTank().SetAttackTarget(pos);
        }
        break;
    case TCI_Turn:
        {
            Point2f turnToPos = *(const Point2f *)pData;
            turnToPos = turnToPos - getTank().pos;
            getTank().setTurnDirection = true;
            getTank().turnDirection = atan2(turnToPos.y, turnToPos.x) * 180.f / Pi;
        }
        break;
    }
}

void Tank::CollisionEntity::onCollision(iCollisionEntity &o, const iContactInfo &contactInfo)
{
    float fDamage;
    int nPenetrability;
    Point2f oPos;

    contactInfo;
    if (getEntity().state == TS_Dead)
        return;

    Entity &oe = o.getEntity();
    switch(o.getCollisionCategory())
    {
    case CC_Tank:
        break;
    case CC_Bullet:
        if (oe.getStatus(Bullet::ESI_Damage, &fDamage) &&
            oe.getStatus(Bullet::ESI_Penetrability, &nPenetrability))
            getEntity().damage(fDamage, 0);
        break;
    case CC_Cannon:
        if (oe.getStatus(CannonBall::ESI_Damage, &fDamage) &&
            oe.getStatus(CannonBall::ESI_Penetrability, &nPenetrability))
            getEntity().damage(fDamage, nPenetrability);
        break;
    case CC_Explosive:
        {
            Explosive &explosive = *(Explosive *)&oe;
            explosive.getDamage(getEntity().pos, fDamage, nPenetrability);
            getEntity().damage(fDamage, nPenetrability);
        }
        break;
    }
}

void Tank::SetBarbette(const Barbette &barbette, const Barbette &gunBarbette)
{
    if (mBarbette)
        delete mBarbette;
    if (mGunBarbette)
        delete mGunBarbette;
    mBarbette = new Barbette::BarbetteInstance(barbette, *this);
    mGunBarbette = new Barbette::BarbetteInstance(gunBarbette, *this);
}

void Tank::SetCannon(const Cannon &cannon)
{
    if (!mCannon)
    {
        mCannon = new Weapon::WeaponInstance(cannon);
    }
    else if (&mCannon->getWeapon() != &cannon)
    {
        delete mCannon;
        mCannon = new Weapon::WeaponInstance(cannon);
    }
    mCannon->chargeAmmo(20);
}

void Tank::SetGun(const Gun &gun)
{
    if (mGun)
    {
        if (&mGun->getWeapon() != &gun)
        {
            delete mGun;
            mGun = new Weapon::WeaponInstance(gun);
        }
    }
    else
        mGun = new Weapon::WeaponInstance(gun);
    mGun->chargeAmmo(100);
}

Entity::iEntity* Tank::getEntityInterface(EntityInterfaceId id)
{
    switch(id)
    {
    case EII_CollisionEntity:
        return &this->collisionEntity;
    case EII_ControlEntity:
        return &this->controlEntity;
    case EII_GraphicEntity:
        return &this->graphicEntity;
    case EII_ViewerEntity:
        return &this->viewerEntity;
    case EII_RadarEntity:
        return &this->radarEntity;
    case EII_TargetEntity:
        return &this->targetEntity;
    }

    return Entity::getEntityInterface(id);
}

bool Tank::getStatus(EntityStatusId id, void *p)
{
    switch(id)
    {
    case ESI_Position:
        *(Point2f *)p = this->pos;
        return true;
    case ESI_Orientation:
        *(float *)p = this->getDirection() / 180.f * Pi;
        return true;
    case ESI_Velocity:
        *(Point2f *)p = this->curVelocity;
        return true;
    case ESI_GunCoolDown:
        if (mGun)
        {
			Weapon::WeaponCondition wc;
			wc.fCurTime = *(float *)p;
            *(float *)p = mGun->getCooldownPercentage(wc);
            return true;
        }
        break;
    case ESI_CannonCoolDown:
        if (mCannon)
        {
			Weapon::WeaponCondition wc;
			wc.fCurTime = *(float *)p;
            *(float *)p = mCannon->getCooldownPercentage(wc);
            return true;
        }
        break;
    case ESI_GunAmmoAmount:
        if (mGun)
        {
            *(size_t *)p = mGun->getAmmoAmount();
            return true;
        }
        break;
    case ESI_CannonAmmoAmount:
        if (mCannon)
        {
            *(size_t *)p = mCannon->getAmmoAmount();
            return true;
        }
        break;
    }
    return false;
}

void Tank::attachCooldownIcon(CooldownIcon *gunCooldown, CooldownIcon *cannonCooldown)
{
    this->gunAttacheeEntity.attach(*gunCooldown);
    gunCooldown->statusId = ESI_GunCoolDown;
    gunCooldown->statusId_ammoAmount = ESI_GunAmmoAmount;
    this->cannonAttacheeEntity.attach(*cannonCooldown);
    cannonCooldown->statusId = ESI_CannonCoolDown;
    cannonCooldown->statusId_ammoAmount = ESI_CannonAmmoAmount;
}

Tank::GraphicEntity::GraphicEntity(Entity &e, cAni::iAnimResManager &arm, hgeFont *_font) : iGraphicEntity(e, arm), font(_font)
{
    animTank = iSystem::GetInstance()->createAnimation(NumTankAnimIds);
    animTank->setAnimData(animResManager.getAnimData("data/tank/idle.xml"), TAI_Idle);
    //animTank.setAnimData(animResManager.getAnimData("data/tank/move"), TAI_Move);
    //animTank.setAnimData(animResManager.getAnimData("data/tank/accelerate"), TAI_Accelerate);
    //animTank.setAnimData(animResManager.getAnimData("data/tank/decelerate"), TAI_Decelerate);

    animBarbette = iSystem::GetInstance()->createAnimation(NumBarbetteAnimIds);
    animBarbette->setAnimData(animResManager.getAnimData("data/tank/barbetteidle.xml"), BAI_Idle);
    //animBarbette->setAnimData(animResManager.getAnimData("data/tank/barbetteturn.xml"), BAI_Turning);
    //animBarbette->setAnimData(animResManager.getAnimData("data/tank/barbettefire.xml"), BAI_Fire);

    animCross = iSystem::GetInstance()->createAnimation();
    animCross->setAnimData(animResManager.getAnimData("data/tank/cross.xml"), 0);

    explodeSmoke = iSystem::GetInstance()->createAnimation();
    explodeSmoke->setAnimData(animResManager.getAnimData("data/tank/smoke.xml"), 0);

}

Tank::GraphicEntity::~GraphicEntity()
{
    iSystem::GetInstance()->release(this->animTank);
    this->animTank = 0;
    iSystem::GetInstance()->release(this->animBarbette);
    this->animBarbette = 0;
    iSystem::GetInstance()->release(this->animCross);
    this->animCross = 0;
    iSystem::GetInstance()->release(this->explodeSmoke);
    this->explodeSmoke = 0;
}
void Tank::GraphicEntity::render(float gameTime, float deltaTime)
{
    deltaTime;

    assert(animTank);

    int timeInFrame = int(gameTime * 60);

    if (trigStartExplodeSmoke)
    {
        trigStartExplodeSmoke = false;
        explodeSmoke->startAnim(timeInFrame);
    }
    if (getEntity().state == Tank::TS_Dead)
    {
        iRenderQueue::getSingleton().render(getEntity().pos, 0, timeInFrame, explodeSmoke, GLI_Bullets);
    }
    else
    {
        iRenderQueue::getSingleton().render(getEntity().pos, -getEntity().fDirection / 180 * M_PI, timeInFrame, animTank, GLI_Tank);

        float barbetteDirection;
        float gunDirection;
        Point2f posAttackTarget1 = getEntity().pos;
        Point2f posAttackTarget2 = getEntity().pos;
        Barbette::BarbetteInstance *mBarbette = getEntity().mBarbette;
        if (mBarbette)
        {
            barbetteDirection = mBarbette->getDirection();
            posAttackTarget1 = mBarbette->getAttackPos();
        }
        else
        {
            barbetteDirection = getEntity().getDirection();
        }
        Barbette::BarbetteInstance *mGunBarbette = getEntity().mGunBarbette;
        if (mGunBarbette)
        {
            gunDirection = mGunBarbette->getDirection();
            posAttackTarget2 = mGunBarbette->getAttackPos();
        }
        else
        {
            gunDirection = getEntity().getDirection();
        }
        iRenderQueue::getSingleton().render(getEntity().pos, -barbetteDirection / 180 * M_PI, timeInFrame, animBarbette, GLI_Tank);


        char buf[128];
        if (getEntity().bShowAim)
        {
            float attackLength1 = (posAttackTarget1 - getEntity().pos).Length();
            getEntity().mCannon->render(gameTime, deltaTime, getEntity().pos, attackLength1, barbetteDirection / 180 * M_PI);

            float attackLength2 = (posAttackTarget2 - getEntity().pos).Length();
            getEntity().mGun->render(gameTime, deltaTime, getEntity().pos, attackLength2, gunDirection / 180 * M_PI);

            iRenderQueue::getSingleton().render(posAttackTarget1, 0, timeInFrame, animCross, GLI_UI);

            sprintf(buf, "%.2f m", attackLength1);
            iRenderQueue::getSingleton().render(posAttackTarget1 + Point2f(0, -20), font, buf, TA_CENTER, ARGB(255, 255, 128, 128), GLI_UI);
        }

        sprintf(buf, "%d", int(ceil(getEntity().fArmor)));
        int dx = (int)font->GetStringWidth(buf);
        iRenderQueue::getSingleton().render(getEntity().pos + Point2f(-dx-2.f, -20), font, buf, TA_LEFT, ARGB(255, 32, 64, 255), GLI_UI);
        sprintf(buf, "%d", int(ceil(getEntity().fHealth)));
        iRenderQueue::getSingleton().render(getEntity().pos + Point2f(2, -20), font, buf, TA_LEFT, ARGB(255, 255, 64, 32), GLI_UI);
    }
#ifdef _DEBUG
#if 0
    const Rectf &r = getEntity().collisionEntity.getBoundingBox();
    iRenderQueue::getSingleton().render(Point2f(r.left, r.top), Point2f(r.right, r.top), ARGB(128, 0, 0, 255));
    iRenderQueue::getSingleton().render(Point2f(r.right, r.top), Point2f(r.right, r.bottom), ARGB(128, 0, 0, 255));
    iRenderQueue::getSingleton().render(Point2f(r.right, r.bottom), Point2f(r.left, r.bottom), ARGB(128, 0, 0, 255));
    iRenderQueue::getSingleton().render(Point2f(r.left, r.bottom), Point2f(r.left, r.top), ARGB(128, 0, 0, 255));
#endif
#endif
}
