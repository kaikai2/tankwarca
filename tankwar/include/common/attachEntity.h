#ifndef COMMON_ATTACH_ENTITY_H
#define COMMON_ATTACH_ENTITY_H

#include <vector>
#include <algorithm>
#include "common/entity.h"

using std::vector;
using std::find;
using std::swap;

class iAttacheeEntity : public Entity::iEntity
{
public:
    friend class Attacher;
    iAttacheeEntity(Entity &e) : Entity::iEntity(e)
    {
    }
    virtual void attach(Attacher &attacher) = 0;
    virtual void dettach(Attacher &attacher) = 0;
protected:
    void _attach(Attacher &attacher);
    void _dettach(Attacher &attacher);
};

class iSingleAttacheeEntity : public iAttacheeEntity
{
public:
    friend class Attacher;
    iSingleAttacheeEntity(Entity &e) : iAttacheeEntity(e), attacher(0)
    {
    }
    virtual ~iSingleAttacheeEntity()
    {
        if (this->attacher)
            dettach(*this->attacher);
    }
    void attach(Attacher &attacher);
    void dettach(Attacher &attacher);
protected:
    Attacher *attacher;
};

class iMultiAttacheeEntity : public iAttacheeEntity
{
public:
    friend class Attacher;
    iMultiAttacheeEntity(Entity &e) : iAttacheeEntity(e)
    {
    }
    virtual ~iMultiAttacheeEntity()
    {
        for (size_t i = 0; i < attachers.size(); i++)
        {
            _dettach(*attachers[i]);
        }
        attachers.clear();
    }
    void attach(Attacher &attacher);
    void dettach(Attacher &attacher);
protected:
    vector<Attacher *> attachers;
};

class Attacher
{
public:
    friend class iAttacheeEntity;
protected:
    virtual void attach(iAttacheeEntity &entity) = 0;
    virtual void dettach(iAttacheeEntity &entity) = 0;
    virtual void OnAttach(iAttacheeEntity &entity)
    {
        entity;
    }
    virtual void OnDettach(iAttacheeEntity &entity)
    {
        entity;
    }
};

// One on One attacher
class SingleAttacher : public virtual Attacher
{
public:
    SingleAttacher() : entity(0)
    {
    }
    virtual ~SingleAttacher()
    {
        if (isAttached())
        {
            entity->dettach(*this);
        }
    }
    bool isAttached() const
    {
        return entity != 0;
    }
    Entity* getEntity()
    {
        if (isAttached())
            return &entity->getEntity();

        return 0;
    }
    const Entity* getEntity() const
    {
        if (isAttached())
            return &entity->getEntity();

        return 0;
    }
protected:
    void attach(iAttacheeEntity &entity)
    {
        assert(this->entity == 0);

        this->entity = &entity;

        OnAttach(entity);
    }
    void dettach(iAttacheeEntity &entity)
    {
        assert(this->entity == &entity);

        OnDettach(entity);

        this->entity = 0;
    }
    iAttacheeEntity* entity;
};

class MultiAttacher : public virtual Attacher
{
public:
    virtual ~MultiAttacher()
    {
        while(!entities.empty())
        {
            entities.front()->dettach(*this);
        }
    }
    size_t getAttachedCount() const
    {
        return entities.size();
    }
protected:
    void attach(iAttacheeEntity &entity)
    {
        if (find(entities.begin(), entities.end(), &entity) != entities.end())
        {
            assert(0);
            return;
        }
        entities.push_back(&entity);

        OnAttach(entity);
    }
    void dettach(iAttacheeEntity &entity)
    {
        OnDettach(entity);

        vector<iAttacheeEntity*>::iterator ie = find(entities.begin(), entities.end(), &entity);
        if (ie == entities.end())
        {
            assert(0);
            return;
        }
        entities.erase(remove(entities.begin(), entities.end(), &entity), entities.end());
    }
    vector<iAttacheeEntity*> entities;
};

inline
void iAttacheeEntity::_attach(Attacher &attacher)
{
    attacher.attach(*this);
}

inline
void iAttacheeEntity::_dettach(Attacher &attacher)
{
    attacher.dettach(*this);
}

inline
void iSingleAttacheeEntity::attach(Attacher &attacher)
{
    if (this->attacher)
        dettach(attacher);
    _attach(attacher);
    this->attacher = &attacher;
}

inline
void iSingleAttacheeEntity::dettach(Attacher &attacher)
{
    assert(this->attacher == &attacher);
    _dettach(attacher);
    this->attacher = 0;
}

inline
void iMultiAttacheeEntity::attach(Attacher &attacher)
{
    if (!this->attachers.empty())
    {
        vector<Attacher*>::iterator ia = find(this->attachers.begin(), this->attachers.end(), &attacher);
        if (ia != this->attachers.end())
        {
            assert(0 && "already attached to the attacher");
            return;
        }
    }
    _attach(attacher);
    this->attachers.push_back(&attacher);
}

inline
void iMultiAttacheeEntity::dettach(Attacher &attacher)
{
    assert(!this->attachers.empty());
    vector<Attacher*>::iterator ia = find(this->attachers.begin(), this->attachers.end(), &attacher);
    if (ia != this->attachers.end())
    {
        _dettach(**ia);
        swap(*ia, this->attachers.back());
        this->attachers.pop_back();
    }
    else
    {
        assert(0 && "not attached to the attacher");
    }
}
#endif//COMMON_ATTACH_ENTITY_H