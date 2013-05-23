#ifndef TANK_GAME_COLLISION_DEF_H
#define TANK_GAME_COLLISION_DEF_H

// 碰撞种类
enum CollisionCategory
{
    CC_Tank             = 0x00000001,   // 普通坦克
    CC_AmphibiousTank   = 0x00000002,   // 两栖坦克
    CC_Bullet           = 0x00000004,   // 子弹
    CC_Cannon           = 0x00000008,   // 炮弹
    CC_Box              = 0x00000010,   // 障碍物
    CC_Wetland          = 0x00000020,   // 湿地
    CC_Item             = 0x00000040,   // 道具
    CC_Explosive        = 0x00000080,   // 爆炸
};

// 各种类与什么种类发生碰撞检测
enum CollisionBits
{
    CB_Tank             = CC_Tank | CC_AmphibiousTank | CC_Box | CC_Explosive,
    CB_AmphibiousTank   = CC_Tank | CC_AmphibiousTank | CC_Box | CC_Explosive,
    CB_Bullet           = CC_Tank | CC_AmphibiousTank | CC_Box | CC_Cannon,
    CB_Cannon           = CC_Tank | CC_AmphibiousTank | CC_Box | CC_Bullet | CC_Cannon,
    CB_Box              = CC_Tank | CC_AmphibiousTank | CC_Box | CC_Bullet | CC_Cannon,
    CB_Wetland          = CC_Tank, // 湿地阻断坦克，但允许两栖坦克通过。
    CB_Item             = CC_Tank | CC_AmphibiousTank,
    CB_Explosive        = CC_Tank | CC_AmphibiousTank,
};

// 哪些碰撞需要做出反应，无反应的情况将会穿透（并且不断触发）
enum CollisionBlockBits
{
    CBB_Tank            = CC_Tank | CC_AmphibiousTank | CC_Box,
    CBB_AmphibiousTank  = CC_Tank | CC_AmphibiousTank | CC_Box,
    CBB_Bullet          = CB_Bullet,
    CBB_Cannon          = CB_Cannon,
    CBB_Box             = CB_Box,
    CBB_Wetland         = CB_Wetland,
    CBB_Item            = 0,
    CBB_Explosive       = 0,
};

#endif