#ifndef _caHgePort_System_H_
#define _caHgePort_System_H_

#include "caHgePort_define.h"

class hgeCurvedAniSystem : public iSystem
{
public:
    hgeCurvedAniSystem()
    {
        assert(!hge);
        hge = hgeCreate(HGE_VERSION);

        attach(&this->renderer);
    }
    virtual ~hgeCurvedAniSystem()
    {
        dettach(&this->renderer);

        // MUST call this here
        iSystem::release();
        hge->Release();
    }
    virtual iByteStream *loadStream(const char *name)
    {
        hgeByteStream *byteStream = new hgeByteStream;
        byteStream->loadByteStream(name);
        return byteStream;
    }
    virtual iCurveInterpolater *createCurveInterpolater(const char *name);
    virtual iStateMergerIf *createStateMerger(const char *name);
    virtual void release(iByteStream *bs);
    virtual void release(iCurveDataSet *cds);
    virtual void release(iCurveInterpolater *ci);
    virtual void release(iStateMergerIf *sm);
    virtual iRenderer *getRenderer()
    {
        return &renderer;
    }
    virtual float getTime()
    {
        return hge->Timer_GetTime();
    }

    /// @fn getPortName
    /// @brief return the name of the port
    virtual const char *getPortName() const
    {
        return "HGE";
    }
    static iSystem* GetInstance()
    {
        return s_Instance;
    }
protected:
    virtual iTexture *allocTexture();
    virtual void release(cAni::iTexture *tex);
    static HGE *hge;
    hgeRenderer renderer;
};

#endif//_caHgePort_System_H_
