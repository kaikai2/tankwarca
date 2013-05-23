#ifndef CONTROL_ENTITY_H
#define CONTROL_ENTITY_H

#include <vector>
#include "common/entity.h"
#include "common/attachEntity.h"

class iControlEntity : public iSingleAttacheeEntity
{
public:
    friend class Controller;
    iControlEntity(Entity &e) : iSingleAttacheeEntity(e)
    {
    }
    virtual ~iControlEntity()
    {
    }
protected:
    virtual void onControllerCommand(int command, const void *pData) = 0;
};

class Controller : public virtual Attacher
{
public:
    virtual void sendCommand(int command, const void *data = 0) = 0;
protected:
    static void sendCommand(iControlEntity &entity, int command, const void *data)
    {
        entity.onControllerCommand(command, data);
    }
};

class SingleController : public SingleAttacher, public Controller
{
public:
    SingleController()
    {
    }
    void sendCommand(int command, const void *data = 0)
    {
        if (entity)
            Controller::sendCommand(*(iControlEntity *)entity, command, data);
    }
};

class MultiController : public MultiAttacher, public Controller
{
public:
    void sendCommand(int command, const void *data = 0)
    {
        for (vector<iAttacheeEntity*>::iterator ie = entities.begin(); ie != entities.end(); ++ie)
            Controller::sendCommand(*(iControlEntity*)*ie, command, data);
    }
};

#endif