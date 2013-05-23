#ifndef Tank_H
#define Tank_H

#include "hge.h"
#include "curvedani.h"
#include "caPoint2d.h"
#include "verlet/verlet.h"

#include "common/controlEntity.h"
#include "common/collisionEntity.h"
#include "common/graphicEntity.h"

#include "common/fsm.h"

#include "gameDef.h"
#include "collisionDef.h"
#include "weapon.h"
#include "radarMap.h"
#include "targetIndicator.h"



using cAni::iSystem;
using cAni::iAnimation;
using cAni::Point2f;

class GameBase;
struct Barbette;
struct Cannon;
struct Gun;
class CooldownIcon;

class Tank : public Entity, public Barbette::BarbetteBase
{
public:
    enum EntityInterfaceIds
    {
        EII_RadarEntity = NumEntityInterfaceIds,
        EII_TargetEntity,
        NumEntityInterfaceId,
    };
    enum TankEntityStatusIds
    {
        ESI_GunCoolDown = NumEntityStatusIds,
        ESI_CannonCoolDown,
        ESI_GunAmmoAmount,
        ESI_CannonAmmoAmount,
        NumEntityStatusId,
    };
    // animation IDs of tank body
    enum TankAnimId
    {
        TAI_Idle,
        TAI_Move,
        TAI_Accelerate,
        TAI_Decelerate,

        NumTankAnimIds,
    };
    // animation IDs of barbette
    enum BarbetteAnimId
    {
        BAI_Idle,
        BAI_Turning,
        BAI_Fire,

        NumBarbetteAnimIds,
    };
    // accepted commands through the controller
    enum TankCommandId
    {
        TCI_TurnLeft,
        TCI_TurnRight,
        TCI_Forward,
        TCI_Backward,
        TCI_HalfEnginePower,
        //TCI_CutEnginePower,
        TCI_Fire, // primary weapon
        TCI_Shoot, // secondary weapon
        TCI_Aim,

        TCI_Turn,

        NumTankCommandIds,
    };
    enum TankState
    {
        TS_Alive,
        TS_Dead,
    } state;

    class ControlEntity : public iControlEntity
    {
    public:
        ControlEntity(Tank &tank) : iControlEntity(tank)
        {
        }
    protected:
        virtual void onControllerCommand(int command, const void *pData);

        Tank &getTank()
        {
            return (Tank &)this->entity;
        }
        const Tank &getTank() const
        {
            return (const Tank &)this->entity;
        }
    };

    Tank(cAni::iAnimResManager &arm, hgeFont *font);
    virtual ~Tank();
    virtual void step(float gameTime, float deltaTime);
    enum MoveStatus
    {
        MS_UP       = 0x01, 
        MS_DOWN     = 0x02,
        MS_LEFT     = 0x04,
        MS_RIGHT    = 0x08,
        MS_HalfEnginePower = 0x10,
    };
    void RemoveMoveStatus(DWORD ms)
    {
        mMoveStatus &= ~ms;
    }
    void AddMoveStatus(DWORD ms)
    {
        mMoveStatus |= ms;
    }
    DWORD mMoveStatus;
    enum AttackStatus
    {
        AS_Cannon = 1,
        AS_Gun = 2,
        // AS_Mine = 4,
    };
    void RemoveAttackStatus(DWORD as)
    {
        mAttackStatus &= ~as;
    }
    void AddAttackStatus(DWORD as)
    {
        mAttackStatus |= as;
    }
    DWORD mAttackStatus;

    Point2f pos;
    bool bShowAim;
    bool bAmphibious;
    float turnDirection;
    bool setTurnDirection;

    virtual float getDirection() const
    {
        float dir = fDirection;
        if (dir < 0)
            dir += 360.0f * int(-dir / 360.0f);
        while(dir < 0)
            dir += 360.0f;
        if (dir > 360.0)
            dir -= 360.0f * int(dir / 360.0f);
        while(dir >= 360.0f)
            dir -= 360.0f;
        return dir;
    }
    float fDirection;
    float fSpeed;
    float fCurSpeed;
    Point2f curVelocity;

    void SetAttackTarget(const Point2f &pos)
    {
        if (mBarbette)
        {
            mBarbette->setAttackTarget(pos);
        }
        if (mGunBarbette)
        {
            mGunBarbette->setAttackTarget(pos);
        }
    }
    void SetBarbette(const Barbette &barbette, const Barbette &gunBarbette);
    void SetCannon(const Cannon &cannon);
    void SetGunBarbette(const Barbette &barbette);
    void SetGun(const Gun &gun);
    Barbette::BarbetteInstance *mBarbette; // 炮塔
    Weapon::WeaponInstance *mCannon; // 主炮
    Barbette::BarbetteInstance *mGunBarbette; // 机枪塔
    Weapon::WeaponInstance *mGun; // 机枪

    void attachCooldownIcon(CooldownIcon *gunCooldown, CooldownIcon *cannonCooldown);
    float fGunDirection;      // 机枪方向
    float fArmor;
    float fHealth;


    virtual iEntity* getEntityInterface(EntityInterfaceId id);
    virtual bool getStatus(EntityStatusId id, void *p);

    virtual EntityTypeId getTypeId() const
    {
        return ETI_Tank;
    }
    float getSize() const
    {
        return 16;
    }
    void damage(float fDamage, int nPenetrability);
protected:

    class CollisionEntity : public iCollisionEntity
    {
    public:
        CollisionEntity(Entity &e) : iCollisionEntity(e)
        {
            verlet::BoxShape *box = new verlet::BoxShape(body, Point2f(getEntity().getSize() / 2, getEntity().getSize() / 2));
            body.SetShape(box);
            resetTransform();
        }
        virtual ~CollisionEntity()
        {
            delete body.GetShape();
            body.SetShape(0);
        }
        void resetTransform()
        {
            verlet::BoxShape *box = (verlet::BoxShape *)body.GetShape();
            assert(box);
            body.Reset(getEntity().pos, getEntity().fDirection / 180.f * Pi(), 5.0f);
            this->setboundingBox(box->GetBoundingBox() + getEntity().pos);
        }
        virtual unsigned long getCollisionCategory() const // 自己的类别标志位
        {
            return getEntity().bAmphibious ? CC_AmphibiousTank : CC_Tank;
        }
        virtual unsigned long getCollisionBits() const // 与哪种类发生碰撞
        {
            return getEntity().bAmphibious ? CB_AmphibiousTank : CB_Tank;
        }
        virtual unsigned long getCollisionBlockBits() const
        {
            return getEntity().bAmphibious ? CBB_AmphibiousTank : CBB_Tank;
        }
        virtual void onCollision(iCollisionEntity &o, const iContactInfo &contactInfo);
        virtual verlet::iBody &getBody()
        {
            return body;
        }
        virtual const verlet::iBody &getBody() const
        {
            return body;
        }
        virtual void onAfterPhysics()
        {
            getEntity().pos = body.getPosition();
            verlet::BoxShape *box = (verlet::BoxShape *)body.GetShape();
            assert(box);
            this->setboundingBox(box->GetBoundingBox() + getEntity().pos);
        }
    protected:
        Tank &getEntity()
        {
            return *(Tank*)&entity;
        }
        const Tank &getEntity() const
        {
            return *(const Tank*)&entity;
        }
        verlet::Body body;
    };
    class GraphicEntity : public iGraphicEntity
    {
    public:
        GraphicEntity(Entity &e, cAni::iAnimResManager &arm, hgeFont *font);
        virtual ~GraphicEntity();
        virtual void render(float gameTime, float deltaTime);
        void playSmoke()
        {
            trigStartExplodeSmoke = true;
        }
        bool checkExplodeSmokeEnd(float curTime) const
        {
            return !explodeSmoke || explodeSmoke->checkEnd(int(curTime * 60));
        }
    protected:
        const Tank &getEntity() const
        {
            return *(const Tank*)&entity;
        }
        iAnimation *animTank;
        iAnimation *animBarbette;
        iAnimation *animCross;
        iAnimation *explodeSmoke;

        hgeFont *font;
        bool trigStartExplodeSmoke;
    };
    class ViewerEntity : public iViewerEntity
    {
    public:
        ViewerEntity(Entity &e) : iViewerEntity(e)
        {
        }
        virtual Point2f getViewerPos() const
        {
            return getEntity().pos;
        }
        virtual float getViewerOrientation() const
        {
            return getEntity().fDirection * Pi() / 180.0f;
        }
    protected:
        const Tank &getEntity() const
        {
            return *(const Tank*)&entity;
        }
    };
    void Fire(const Weapon::WeaponCondition &rWeaponCondition); // 主炮 开火
    void Shoot(const Weapon::WeaponCondition &rWeaponCondition);// 机枪 开火

    CollisionEntity collisionEntity;
    ControlEntity controlEntity;
    GraphicEntity graphicEntity;
    ViewerEntity viewerEntity;
    iRadarEntity radarEntity;
    iTargetEntity targetEntity;
    iSingleAttacheeEntity gunAttacheeEntity;
    iSingleAttacheeEntity cannonAttacheeEntity;

	Fsm fsmMove;
};

#endif//Tank_H
