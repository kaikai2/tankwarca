#ifndef Weapon_H
#define Weapon_H

#include "common/utils.h"
#include "caPoint2d.h"
using cAni::Point2f;

struct ShopGoods
{
    ShopGoods(const char *_name, int price) : 
        name(_name), nPrice(price)
    {
    }
    const char * const name; // 名称
    const int nPrice; // 买入价格

private:
    ShopGoods &operator = (const ShopGoods& o)
    {
        o;
        return *this;
    }
};

struct ExplosiveInfo// : public NoCopy
{
    ExplosiveInfo(const float _fDamage, const int _nPenetrability, const float _fDamageRange) : 
        fDamage(_fDamage), nPenetrability(_nPenetrability), fDamageRange(_fDamageRange)
    {
    }
    const float fDamage; // 杀伤力
    const int nPenetrability; // 穿透力，针对装甲厚度 Barbette::nArmor
    const float fDamageRange; // 杀伤半径
};

struct Weapon : public ShopGoods
{
	struct WeaponCondition
	{
		float fCurTime;
	};

    Weapon(const char *_name, int price, int _nGrade, 
        float _fShootRange, float _fReloadDelay, float _fDefinition, float _fDamage, float _fMuzzleVelocity, int _nPenetrability, const ExplosiveInfo * const _explosive = 0) : 
        ShopGoods(_name, price), nGrade(_nGrade), fShootRange(_fShootRange), fReloadDelay(_fReloadDelay),
        fDefinition(_fDefinition), fDamage(_fDamage), fMuzzleVelocity(_fMuzzleVelocity), nPenetrability(_nPenetrability),
        explosive(_explosive)
    {
    }
    const int nGrade; // 需要炮塔等级
    const float fShootRange; // 射程 m
    const float fReloadDelay; // 装填时间 sec
    const float fDefinition; // 精确度（相对目标误差/距离 m/m）
    const float fDamage; // 杀伤力
    const float fMuzzleVelocity; // 初速度
    const int nPenetrability; // 穿透力，针对装甲厚度 Barbette::nArmor
    const ExplosiveInfo * const explosive; // 爆炸时的数据

    class WeaponInstance : public NoCopy
    {
    public:
        WeaponInstance(const Weapon &_weapon) : weapon(_weapon), fCooldownWait(0), amount(0)
        {
        }
        void render(float fGameTime, float fDeltaTime, const Point2f &pos, float attackLen, float orientation);
        bool isReadyToFire(const WeaponCondition &rCondition) const
        {
            return amount > 0 && rCondition.fCurTime > fCooldownWait;
        }
        void fire(const WeaponCondition &rCondition)
        {
            assert(isReadyToFire(rCondition));
            --amount;
            fCooldownWait = rCondition.fCurTime + weapon.fReloadDelay;
        }
        virtual const Weapon &getWeapon() const 
        {
            return weapon;
        }
        float getCooldownPercentage(const WeaponCondition &rCondition) const
        {
			return clamp(1.0f - (fCooldownWait - rCondition.fCurTime) / weapon.fReloadDelay, 0.f, 1.f);
        }
        void chargeAmmo(size_t amount)
        {
            this->amount += amount;
        }
        size_t getAmmoAmount() const
        {
            return amount;
        }
    protected:
        const Weapon &weapon;
        float fCooldownWait;
        size_t amount;
    };
};

struct Cannon : public Weapon
{
    Cannon(const char *_name, int price, int _nGrade, 
        float _fShootRange, float _fReloadDelay, float _fDefinition, float _fDamage, float _fMuzzleVelocity, int _nPenetrability,
        const ExplosiveInfo * const _explosive) :
        Weapon(_name, price, _nGrade, _fShootRange, _fReloadDelay, _fDefinition, _fDamage, _fMuzzleVelocity, _nPenetrability, _explosive)
    {
    }
    static Cannon* GetData(int index);
};

struct Gun : public Weapon
{
    Gun(const char *_name, int price, int _nGrade, 
        float _fShootRange, float _fReloadDelay, float _fDefinition, float _fDamage, float _fMuzzleVelocity, int _nPenetrability) :
        Weapon(_name, price, _nGrade, _fShootRange, _fReloadDelay, _fDefinition, _fDamage, _fMuzzleVelocity, _nPenetrability)
    {
    }

    static Gun* GetData(int index);
};

struct Barbette : public ShopGoods
{
    Barbette(const char *_name, int price, 
        int _cannonGradeCapacity, int _gunGradeCapacity, float _fSightRange, float _fDirAngleRange, float _fMoveSpeed, int _nArmor) :
        ShopGoods(_name, price), 
        cannonGradeCapacity(_cannonGradeCapacity), gunGradeCapacity(_gunGradeCapacity), 
        fSightRange(_fSightRange), fDirAngleRange(_fDirAngleRange), fMoveSpeed(_fMoveSpeed), nArmor(_nArmor)
    {
    }
    const int cannonGradeCapacity; // 主炮装载能力
    const int gunGradeCapacity; // 装载能力
    const float fSightRange; // 观察半径 m
    const float fDirAngleRange; // 主炮攻击角度 ang
    const float fMoveSpeed; // 炮塔旋转相对速度 ang / sec
    const int nArmor; // 装甲厚度, 防御

    class BarbetteBase
    {
    public:
        virtual float getDirection() const = 0;
    protected:
    };
    class BarbetteInstance : public NoCopy, public BarbetteBase
    {
    public:
        BarbetteInstance(const Barbette &_barbette, const BarbetteBase &_base)
            : barbette(_barbette), base(_base), fBarbetteDirection(0)
        {
        }
        void step(float fGameTime, float fDeltaTime);
        void render(float fGameTime, float fDeltaTime);
        void setAttackTarget(const Point2f &pos)
        {
            posAttackTarget = pos;
        }
        void setCenterPos(const Point2f &pos)
        {
            posCenter = pos;
        }
        virtual float getDirection() const
        {
            float dir = fBarbetteDirection + base.getDirection();
            while(dir < 0)
                dir += 360.0f;
            while(dir >= 360.0f)
                dir -= 360.0f;
            return dir;
        }
        const Point2f& getAttackPos() const
        {
            return posAttackTarget;
        }
    protected:
        const Barbette &barbette;
        const BarbetteBase &base;
        Point2f posCenter;
        Point2f posAttackTarget;
        float fBarbetteDirection; // 炮塔方向
    };
    static Barbette* GetData(int index);
};
struct Enginee : public ShopGoods
{
    Enginee(const char *_name, int price, float _fPower, float _fMaxAcceleration) :
        ShopGoods(_name, price), fPower(_fPower), fMaxAcceleration(_fMaxAcceleration)
    {
    }
    const float fPower; // 动力输出
    const float fMaxAcceleration; // 加速度上限

    static Enginee* GetData(int index);
};


#endif