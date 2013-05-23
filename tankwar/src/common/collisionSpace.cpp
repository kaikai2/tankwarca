#include <vector>
#include <algorithm>
#include "common/entityManager.h"
#include "common/profile.h"
#include "common/iRenderQueue.h"
#include "common/collisionSpace.h"

using std::vector;
using std::find;

void Culler::cull(CollisionSpace &space, const EntityFilter &ef)
{
    space.update();
    space.cull(*this, ef);
}

size_t CollisionSpace::checkInnerCollision(CollisionChecker &collisionChecker, iContactInfo &contactInfo)
{
    assert(newEntities.empty());
    size_t collisionCount = 0;
    size_t size = entities.size();
    for (size_t i = 0; i < size; ++i)
    {
        iCollisionEntity *e = entities[i];
        const unsigned long category = e->getCollisionCategory();
        const unsigned long collision = e->getCollisionBits();
        if ((category | collision) == 0)
            continue;

        const unsigned long blockBits = e->getCollisionBlockBits();
        const vector<const iCollisionEntity*> &ies1 = e->getIgnoredEntities();
        for (size_t j = i + 1; j < size; ++j)
        {
            iCollisionEntity *e2 = entities[j];

            const unsigned long category2 = e2->getCollisionCategory();
            const unsigned long collision2 = e2->getCollisionBits();
            if (!(category & collision2) && !(category2 & collision))
                continue;

            if (!ies1.empty() && find(ies1.begin(), ies1.end(), e2) != ies1.end())
                continue;

            const vector<const iCollisionEntity*> &ies2 = e2->getIgnoredEntities();

            if (!ies2.empty() && find(ies2.begin(), ies2.end(), e) != ies2.end())
                continue;

            if (collisionChecker.checkCollision(e, e2, contactInfo))
            {
                const unsigned long blockBits2 = e2->getCollisionBlockBits();
                collisionCount++;
                if ((category & blockBits2) || (category2 & blockBits))
                    contactInfo.Update();
                e->onCollision(*e2, contactInfo);
                e2->onCollision(*e, contactInfo);
            }
        }

        if (child)
        {
            const Rectf &r = e->getBoundingBox();
            size_t cid = getChildId(r);
            if (cid < NumSubSpaces)
            {
                assert(0); // should not be here
                collisionCount += getChild(cid).checkCollision(e, collisionChecker, contactInfo);
            }
            else
            {
                assert(cid == size_t(-1));
                for (cid = 0; cid < NumSubSpaces; cid++)
                {
                    if (getChild(cid).isIntersected(r))
                        collisionCount += getChild(cid).checkCollision(e, collisionChecker, contactInfo);
                }
            }
        }
    }
    if (child)
    {
        collisionCount += getChild(LeftTop).checkInnerCollision(collisionChecker, contactInfo)
            + getChild(LeftBottom).checkInnerCollision(collisionChecker, contactInfo)
            + getChild(RightTop).checkInnerCollision(collisionChecker, contactInfo)
            + getChild(RightBottom).checkInnerCollision(collisionChecker, contactInfo);
    }
    return collisionCount;
}

size_t CollisionSpace::checkCollision(iCollisionEntity *e, CollisionChecker &collisionChecker, iContactInfo &contactInfo)
{
    assert(e);
    const unsigned long category = e->getCollisionCategory();
    const unsigned long collision = e->getCollisionBits();
    if ((category | collision) == 0)
        return 0;

    const unsigned long blockBits = e->getCollisionBlockBits();
    size_t collisionCount = 0;
    const vector<const iCollisionEntity*> &ies1 = e->getIgnoredEntities();

    size_t size = entities.size();
    for (size_t i = 0; i < size; ++i)
    {
        iCollisionEntity *e2 = entities[i];

        const unsigned long category2 = e2->getCollisionCategory();
        const unsigned long collision2 = e2->getCollisionBits();

        if (!(category & collision2) && !(category2 & collision))
            continue;

        if (!ies1.empty() && find(ies1.begin(), ies1.end(), e2) != ies1.end())
            continue;

        const vector<const iCollisionEntity*> &ies2 = e2->getIgnoredEntities();

        if (!ies2.empty() && find(ies2.begin(), ies2.end(), e) != ies2.end())
            continue;

        if (collisionChecker.checkCollision(e, e2, contactInfo))
        {
            const unsigned long blockBits2 = e2->getCollisionBlockBits();
            collisionCount++;
            if ((category & blockBits2) || (category2 & blockBits))
                contactInfo.Update();
            e->onCollision(*e2, contactInfo);
            e2->onCollision(*e, contactInfo);
        }
    }

    if (child)
    {
        const Rectf &r = e->getBoundingBox();

        size_t cid = getChildId(r);
        if (cid < NumSubSpaces)
        {
            collisionCount += getChild(cid).checkCollision(e, collisionChecker, contactInfo);
        }
        else
        {
            assert(cid == size_t(-1));
            for (cid = 0; cid < NumSubSpaces; cid++)
            {
                if (getChild(cid).isIntersected(r))
                    collisionCount += getChild(cid).checkCollision(e, collisionChecker, contactInfo);
            }
        }
    }

    return collisionCount;
}

void CollisionSpace::update()
{
#ifdef USE_RESPLIT
    PROFILE_BEGINPOINT(collisionSpace.updateInner1);
    NewCollisionEntities changedCollisionEntities;
    vector<CollisionSpace *> candidates;
    candidates.reserve(d * 4);
    candidates.push_back(this);
    while(!candidates.empty())
    {
        CollisionSpace *node = candidates.back();
        candidates.pop_back();
        if (!node->newEntities.empty())
        {
            changedCollisionEntities.insert(changedCollisionEntities.end(), node->newEntities.begin(), node->newEntities.end());
            node->newEntities.clear();
        }
#if 1
        if (!node->entities.empty())
        {
            for (size_t i = 0; i < node->entities.size();)
            {
                iCollisionEntity *entity = node->entities[i];
                assert(entity && entity->space == node);
                if (entity->isBoundingBoxChanged())
                {
                    changedCollisionEntities.push_back(entity);
                    std::swap(node->entities.back(), node->entities[i]);
                    node->entities.pop_back();
                }
                else
                    ++i;
            }
        }
#else
        for (CollisionEntities::iterator ie = node->entities.begin(); ie != node->entities.end();)
        {
            assert((*ie)->space == node);
            if ((*ie)->isBoundingBoxChanged())
            {
                changedCollisionEntities.push_back(*ie);
                ie = node->entities.erase(ie);
            }
            else
                ++ie;
        }
#endif
        if (node->child)
        {
            for (size_t i = 0; i < NumSubSpaces; i++)
            {
                candidates.push_back(&node->getChild(i));
            }
        }
    }
    PROFILE_CHECKPOINT(collisionSpace.updateInner2);
#if 1
    size_t size = changedCollisionEntities.size();
    for (size_t i = 0; i < size; ++i)
    {
        iCollisionEntity *ce = changedCollisionEntities[i];
#else
    for (NewCollisionEntities::iterator it = changedCollisionEntities.begin();
        it != changedCollisionEntities.end(); ++it)
    {
        iCollisionEntity *ce = *it;
#endif
        CollisionSpace *space = ce->space;
        const Rectf &r = ce->getBoundingBox();
        ce->clearBoundingBoxChanged();
        for(;;)
        {
            size_t cid;
            if (!space->isContained(r))
            {
                if (space->parent)
                    space = &space->getParent();
                else
                {
                    //assert(space != this);
                    space->entities.push_back(ce);
                    ce->space = space;
                    break;
                }
            }
            else if (space->child && (cid = space->getChildId(r)) < NumSubSpaces)
            {
                space = &space->getChild(cid);
            }
            else
            {
                //assert(space != this);
                space->entities.push_back(ce);
                ce->space = space;
                break;
            }
        }
    }
    PROFILE_ENDPOINT();
#else
    size_t a;
    do
    {
        //PROFILE_CHECKPOINT(collisionSpace.updateInner);
        a = updateInner();
        //PROFILE_ENDPOINT();
    } while(a > 0);
#endif
}
#ifndef USE_RESPLIT
size_t CollisionSpace::updateInner()
{
    vector<iCollisionEntity *>::iterator ie;
    if (!newEntities.empty())
    {
        entities.insert(entities.end(), newEntities.begin(), newEntities.end());
        newEntities.clear();
    }
    for (ie = entities.begin(); ie != entities.end();)
    {
        //ie = entities.begin() + from;
        const Rectf &r = (*ie)->getBoundingBox();
        if (r.left <= range.left || r.right >= range.right || r.top <= range.top || r.bottom >= range.bottom)
        {
            if (parent)
            {
                parent->addEntity(*ie);
                ie = entities.erase(ie);
                //entities.erase(remove(ie, entities.end(), *ie), entities.end());
                continue;
            }
        }
        else
        {
            size_t cid = getChildId(r);
            if (cid < NumSubSpaces)
            {
                if (child)
                {
                    getChild(cid).addEntity(*ie);
                    ie = entities.erase(ie);
                    //entities.erase(remove(ie, entities.end(), *ie), entities.end());
                    continue;
                }
            }
            else
            {
                assert(cid == size_t(-1));
            }
        }
        ++ie;
    }
    assert(checkUpdateSplit());
    size_t count = 0;
    if (child)
    {
        for (int i = 0; i < NumSubSpaces; i++)
        {
            count += getChild(i).updateInner(); // 
        }
    }
    return count += newEntities.size();
}
#endif

void CollisionSpace::removeEntityAll(iCollisionEntity *e)
{
    entities.erase(remove(entities.begin(), entities.end(), e), entities.end());
    //entities.remove(e);
    newEntities.erase(remove(newEntities.begin(), newEntities.end(), e), newEntities.end());
    //newEntities.remove(e);
    if (child)
    {
        for (int i = 0; i < NumSubSpaces; i++)
        {
            getChild(i).removeEntityAll(e);
        }
    }
}
void CollisionSpace::removeEntity(iCollisionEntity *e)
{
    assert(e && e->space);
    CollisionSpace *space = e->space;
    e->space = 0;
    space->entities.erase(remove(space->entities.begin(), space->entities.end(), e), space->entities.end());
    //entities.remove(e);
    if (!space->newEntities.empty())
    {
        space->newEntities.erase(remove(space->newEntities.begin(), space->newEntities.end(), e), space->newEntities.end());
    }
}
#include <hge.h>
void CollisionSpace::renderDebug(float alpha)
{
    if (child)
    {
        iRenderQueue::getSingleton().render(Point2f(range.leftTop.x, center.y), Point2f(center.x, center.y), ARGB(255 * alpha, 255, 255, 255));
        iRenderQueue::getSingleton().render(Point2f(center.x, range.leftTop.y), Point2f(center.x, center.y), ARGB(255 * alpha, 255, 255, 255));

        getChild(LeftTop).renderDebug(alpha * 0.8f);
        getChild(LeftBottom).renderDebug(alpha * 0.8f);
        getChild(RightTop).renderDebug(alpha * 0.8f);
        getChild(RightBottom).renderDebug(alpha * 0.8f);
    }
    for (CollisionEntities::iterator ie = entities.begin(); ie != entities.end(); ++ie)
    {
        const Rectf &r = (*ie)->getBoundingBox();
        iRenderQueue::getSingleton().render(center, Point2f(r.leftTop.x, r.leftTop.y), ARGB(255 * alpha, 255, 0, 0));
    }
}

void CollisionSpace::cull(Culler &culler, const EntityFilter &ef) const
{
    size_t size = entities.size();
    for (size_t i = 0; i < size; ++i)
    {
        iCollisionEntity *e2 = entities[i];
        const Rectf &r = e2->getBoundingBox();
        if ((r & culler.viewer).Visible() && ef.filtrate(e2->getEntity()))
        {
            culler.entities.push_back(&e2->getEntity());
        }
    }
    if (child)
    {
        size_t cid = getChildId(culler.viewer);
        if (cid < NumSubSpaces)
        {
            getChild(cid).cull(culler, ef);
        }
        else
        {
            assert(cid == size_t(-1));
            for (cid = 0; cid < NumSubSpaces; cid++)
            {
                if (getChild(cid).isIntersected(culler.viewer))
                    getChild(cid).cull(culler, ef);
            }
        }
    }
}
