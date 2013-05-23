#include <vector>
#include <algorithm>
#include "common/entityManager.h"
#include "common/profile.h"
using std::vector;
using std::find;


void EntityManager::clear()
{
    bModifyLock = true;
    
    for (size_t i = 0; i < em_entities.size(); ++i)
    {
        delete em_entities[i];
    }
    //this->bodies.clear();
    em_entities.clear();
    bModifyLock = false;
}

void EntityManager::step(float gameTime, float deltaTime)
{
    size_t i;
    bModifyLock = true;
    if (!newComingEntities.empty())
    {
        PROFILE_BEGINPOINT(addNewEntity);
        em_entities.insert(em_entities.end(), newComingEntities.begin(), newComingEntities.end());
        for (i = 0; i < newComingEntities.size(); ++i)
        {
            iCollisionEntity *collEntity = (iCollisionEntity *)newComingEntities[i]->getEntityInterface(Entity::EII_CollisionEntity);
            if (collEntity)
            {
                collisionSpace.addEntity(collEntity);
            }
        }
        newComingEntities.clear();
        PROFILE_ENDPOINT();
    }
    PROFILE_BEGINPOINT(stepAllEntities);
    for (i = 0; i < em_entities.size(); ++i)
    {
        if (em_entities[i]->isActive())
            em_entities[i]->step(gameTime, deltaTime);
    }
    PROFILE_CHECKPOINT(collisionSpace.update);
    collisionSpace.update();
    PROFILE_CHECKPOINT(afterStepAllEntities);
    for (i = 0; i < em_entities.size();)
    {
        if (em_entities[i]->canRelease())
        {
            iCollisionEntity *collEntity = (iCollisionEntity *)em_entities[i]->getEntityInterface(Entity::EII_CollisionEntity);
            if (collEntity)
            {
//                PROFILE_BEGINPOINT(RemoveEntity);
                collisionSpace.removeEntity(collEntity);
//                PROFILE_ENDPOINT();
            }
            delete em_entities[i];
            std::swap(em_entities[i], em_entities.back());
            em_entities.pop_back();
        }
        else
            ++i;
    }
    PROFILE_CHECKPOINT(CheckCollision);
    checkCollision();
    PROFILE_CHECKPOINT(call_onAfterPhysics);
    for (i = 0; i < em_entities.size(); ++i)
    {
        iCollisionEntity *ce = (iCollisionEntity *)em_entities[i]->getEntityInterface(Entity::EII_CollisionEntity);
        if (ce)
        {
            ce->onAfterPhysics();
        }
    }
    bModifyLock = false;
    PROFILE_ENDPOINT();
}

void EntityManager::render(float gameTime, float deltaTime) 
{
    PROFILE_BEGINPOINT(EntityManager::render);
    bModifyLock = true;
#if 0
    Culler culler;
    culler.viewer.leftTop = iRenderQueue::getSingleton().getViewerPos();// + Point2f(100, 100);
    culler.viewer.rightBottom = culler.viewer.leftTop + Point2f(800, 600);
    culler.cull(this->collisionSpace); // cull by collision space, 会遗漏没有iCollisionEntity的Entity
    for (size_t i = 0; i < culler.entities.size(); ++i)
    {
        iGraphicEntity *graphicEntity = (iGraphicEntity *)culler.entities[i]->getEntityInterface(Entity::EII_GraphicEntity);
        if (graphicEntity)
        {
            graphicEntity->render(gameTime, deltaTime);
        }
    }
#else
    for (size_t i = 0; i < em_entities.size(); ++i)
    {
        iGraphicEntity *graphicEntity = (iGraphicEntity *)em_entities[i]->getEntityInterface(Entity::EII_GraphicEntity);
        if (graphicEntity)
        {
            graphicEntity->render(gameTime, deltaTime);
        }
    }
#endif
    PROFILE_ENDPOINT();
//#ifdef _DEBUG
//  this->collisionSpace.renderDebug(0.8f);
//#endif
    bModifyLock = false;
}

void EntityManager::attach(Entity *entity)
{
    assert(entity);
    if (!bModifyLock)
    {
        iCollisionEntity *collEntity = (iCollisionEntity *)entity->getEntityInterface(Entity::EII_CollisionEntity);
        if (collEntity)
        {
            collisionSpace.addEntity(collEntity);
        }
        this->em_entities.push_back(entity);
    }
    else
    {
        this->newComingEntities.push_back(entity);
    }
}

void EntityManager::dettach(Entity *entity)
{
    assert(!bModifyLock);
    iCollisionEntity *collEntity = (iCollisionEntity *)entity->getEntityInterface(Entity::EII_CollisionEntity);
    if (collEntity)
    {
        collisionSpace.update();
        collisionSpace.removeEntity(collEntity);
    }
    
    this->em_entities.erase(remove(this->em_entities.begin(), this->em_entities.end(), entity), this->em_entities.end());
}

void EntityManager::dettach(vector<Entity *> &_entities)
{
    assert(!bModifyLock);
    size_t i;

    vector<iCollisionEntity *> collisionEntities;
    for (i = 0; i < _entities.size(); i++)
    {
        iCollisionEntity *collEntity = (iCollisionEntity *)_entities[i]->getEntityInterface(Entity::EII_CollisionEntity);
        if (collEntity)
        {
            collisionEntities.push_back(collEntity);
        }
        this->em_entities.erase(remove(this->em_entities.begin(), this->em_entities.end(), _entities[i]), this->em_entities.end());
    }
    collisionSpace.update();
    for (i = 0; i < collisionEntities.size(); i++)
    {
        collisionSpace.removeEntity(collisionEntities[i]);
    }
}

void EntityManager::checkCollision()
{
    if (!collisionChecker)
        return;

    iContactInfo &contactInfo = getContactInfoBuffer();
    bModifyLock = true;
    int n = 1;
    do
    {
        PROFILE_BEGINPOINT(collisionSpaceUpdate);
        collisionSpace.update();
        assert(collisionSpace.checkUpdate());
        PROFILE_ENDPOINT();
        PROFILE_BEGINPOINT(checkInnerCollision);
        if (collisionSpace.checkInnerCollision(*collisionChecker, contactInfo) == 0)
        {
            PROFILE_ENDPOINT();
            break;
        }

        PROFILE_CHECKPOINT(call_onAfterPhysics);

        for (size_t i = 0; i < em_entities.size(); i++)
        {
            iCollisionEntity *ce = (iCollisionEntity *)em_entities[i]->getEntityInterface(Entity::EII_CollisionEntity);
            if (ce)
            {
                ce->onAfterPhysics();
            }
        }
        PROFILE_ENDPOINT();
    } while(--n > 0);
    bModifyLock = false;
}

void EntityManager::getEntities(vector<Entity *> &_entities, const EntityFilter &filter)
{
    Culler culler(_entities);
    culler.viewer = filter.getBoundingBox();
    culler.cull(this->collisionSpace, filter);
}
