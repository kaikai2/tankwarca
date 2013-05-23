#ifndef TANK_GAME_COLLISION_DEF_H
#define TANK_GAME_COLLISION_DEF_H

// ��ײ����
enum CollisionCategory
{
    CC_Tank             = 0x00000001,   // ��̹ͨ��
    CC_AmphibiousTank   = 0x00000002,   // ����̹��
    CC_Bullet           = 0x00000004,   // �ӵ�
    CC_Cannon           = 0x00000008,   // �ڵ�
    CC_Box              = 0x00000010,   // �ϰ���
    CC_Wetland          = 0x00000020,   // ʪ��
    CC_Item             = 0x00000040,   // ����
    CC_Explosive        = 0x00000080,   // ��ը
};

// ��������ʲô���෢����ײ���
enum CollisionBits
{
    CB_Tank             = CC_Tank | CC_AmphibiousTank | CC_Box | CC_Explosive,
    CB_AmphibiousTank   = CC_Tank | CC_AmphibiousTank | CC_Box | CC_Explosive,
    CB_Bullet           = CC_Tank | CC_AmphibiousTank | CC_Box | CC_Cannon,
    CB_Cannon           = CC_Tank | CC_AmphibiousTank | CC_Box | CC_Bullet | CC_Cannon,
    CB_Box              = CC_Tank | CC_AmphibiousTank | CC_Box | CC_Bullet | CC_Cannon,
    CB_Wetland          = CC_Tank, // ʪ�����̹�ˣ�����������̹��ͨ����
    CB_Item             = CC_Tank | CC_AmphibiousTank,
    CB_Explosive        = CC_Tank | CC_AmphibiousTank,
};

// ��Щ��ײ��Ҫ������Ӧ���޷�Ӧ��������ᴩ͸�����Ҳ��ϴ�����
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