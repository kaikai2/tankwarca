#ifndef _GAME_TankContactInfo_H_
#define _GAME_TankContactInfo_H_

#include "common/entityManager.h"

class TankContactInfo : public iContactInfo
{
public:
    verlet::Contact *getData()
    {
        return contact;
    }
    const verlet::Contact *getData() const
    {
        return contact;
    }
    void setNumber(size_t num)
    {
        this->num = num;
    }
    size_t getNumber() const
    {
        return num;
    }
    virtual void Update()
    {
        static Point2f off[4] =
        {
            Point2f(-3, -3),
            Point2f(3, -3),
            Point2f(3, 3),
            Point2f(-3, 3),
        };
        for(size_t i = 0; i < num; i++)
        {
            //iRenderQueue::getSingleton().render(contact[i].m_xContacts[0] + off[0], contact[i].m_xContacts[1] + off[2], ARGB(255, 0, 255, 0));
            //iRenderQueue::getSingleton().render(contact[i].m_xContacts[0] + off[1], contact[i].m_xContacts[1] + off[3], ARGB(255, 0, 255, 0));
            contact[i].ResolveOverlap();
            contact[i].ResolveCollision();
        }
    }
protected:
    verlet::Contact contact[8];
    size_t num;

};
class TankCollisionChecker : public CollisionChecker
{
public:
    bool checkCollision(iCollisionEntity *a, iCollisionEntity *b, iContactInfo &contactInfo);

    int checkCount;
    //protected:
    float deltaTime;
};

#endif//_GAME_TankContactInfo_H_
