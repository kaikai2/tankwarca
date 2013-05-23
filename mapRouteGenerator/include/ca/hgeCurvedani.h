#ifndef HGE_CURVEDANI_H
#define HGE_CURVEDANI_H

#include "hge.h"
#include "curvedani.h"
#include "caImage.h"
#include "hgevector.h"
#include "caPoint3d.h"
#include "caRect.h"

#include <vector>
using namespace std;

using cAni::StateId;
using cAni::iTexture;
using cAni::iCurveDataSet;
using cAni::iCurveInterpolater;
using cAni::iClipState;
using cAni::iRenderer;
using cAni::iByteStream;
using cAni::iSystem;
using cAni::Image;
using cAni::Point3f;
using cAni::Rect;


enum hgeCurvedAniStateId
{
    hgeSI_Image,    // size_t
    hgeSI_Position, // vector2
    hgeSI_Scale,    // vector2
    hgeSI_Angle,    // float
    hgeSI_Alpha,    // float
    hgeSI_Color,    // vector3
    hgeSI_ClipRect, // Rect(short4)
    hgeSI_AnchorOffPos, // vector2
};

/// 实现下列接口
/// 纹理提供者
class hgeTexture : public iTexture
{
public:
    hgeTexture() : tex(0)
    {
        if (textureCount++ == 0)
        {
            hge = hgeCreate(HGE_VERSION);
        }
    }
    virtual ~hgeTexture()
    {
        if (tex)
        {
            hge->Texture_Free(tex);
        }
        if (--textureCount == 0)
        {
            hge->Release();
        }
    }
    virtual size_t getWidth() const
    {
        return size_t(hge->Texture_GetWidth(tex));
    }
    virtual size_t getHeight() const
    {
        return size_t(hge->Texture_GetHeight(tex));
    }
    HTEXTURE getHgeTexture()
    {
        if (!tex)
        {
            loadTexture(this->getFilename());
        }
        return tex;
    }
    void loadTexture(const char *filename)
    {
        if (tex)
        {
            hge->Texture_Free(tex);
        }
        tex = hge->Texture_Load(filename);
    }
protected:
    HTEXTURE tex;
    static HGE *hge;
    static size_t textureCount;
};

// 数据集
template<class T>
class hgeCurveDataSet : public iCurveDataSet
{
public:
    virtual const char *getTypeName() const;
    virtual bool loadFromByteStream(iByteStream *bs)
    {
        return iCurveDataSet::loadFromByteStream(bs);
    }
    virtual bool saveToByteStream(iByteStream *bs) const
    {
        return iCurveDataSet::saveToByteStream(bs);
    }
    virtual size_t getDataCount() const
    {
        return data.size();
    }
    virtual void resize(size_t index)
    {
        data.resize(index);
    }
    virtual void setData(size_t index, const void *value, int time)
    {
        assert(index < data.size());
        data[index].data = *(T*)value;
        data[index].time = time;
    }
    virtual void setData(size_t index, const void *value)
    {
        assert(index < data.size());
        data[index].data = *(T*)value;
    }
    virtual void setTime(size_t index, int time)
    {
        assert(index < data.size());
        data[index].time = time;
    }
    virtual int getTime(size_t index) const
    {
        assert(index < data.size());
        return data[index].time;
    }
    virtual const void *getData(size_t index) const
    {
        assert(index < data.size());
        return &data[index].data;
    }
    virtual size_t getDataSize() const
    {
        return sizeof(T);
    }
    virtual void setDataString(size_t index, const char *valueStr);
    virtual const char *getDataString(size_t index) const;
    struct DataNode
    {
        int time;
        T data;
    };
    vector<DataNode> data;
};

/// 参数
class hgeClipState : public iClipState
{
public:
    hgeClipState() : image(), position(0, 0), scale(1, 1), angle(0), alpha(1), color(1, 1, 1), clipRect(), anchorOffPos(0, 0)
    {
    }
    virtual void reset(const iClipState *defaultClipState = 0)
    {
        if (defaultClipState)
        {
            defaultClipState->getState(hgeSI_Image,    &image);
            defaultClipState->getState(hgeSI_Position, &position);
            defaultClipState->getState(hgeSI_Scale,    &scale);
            defaultClipState->getState(hgeSI_Angle,    &angle);
            defaultClipState->getState(hgeSI_Alpha,    &alpha);
            defaultClipState->getState(hgeSI_Color,    &color);
            defaultClipState->getState(hgeSI_ClipRect, &clipRect);
            defaultClipState->getState(hgeSI_AnchorOffPos, &anchorOffPos);
        }
        else
        {
            image = Image();
            position = hgeVector(0, 0);
            scale = hgeVector(1, 1);
            angle = 0;
            alpha = 1;
            color = Point3f(1, 1, 1);
            clipRect = Rect();
            anchorOffPos = hgeVector(0, 0);
        }
    }
    virtual StateId StringToStateId(const char *name)
    {
        return name, 0;
    }
    virtual void *getStateAddr(StateId sid)
    {
        switch(sid)
        {
        case hgeSI_Image:    // size_t
            return &image;
        case hgeSI_Position: // vector2
            return &position;
        case hgeSI_Scale:    // vector2
            return &scale;
        case hgeSI_Angle:    // float
            return &angle;
        case hgeSI_Alpha:    // float
            return &alpha;
        case hgeSI_Color:    // vector3
            return &color;
        case hgeSI_ClipRect: // Rect(short4)
            return &clipRect;
        case hgeSI_AnchorOffPos: // vector2
            return &anchorOffPos;
        default:
            assert(0);
            break;
        }
        return 0;
    }
    virtual void setState(StateId sid, const void *value)
    {
        switch(sid)
        {
        case hgeSI_Image:    // size_t
            image = *(Image*)value;
            break;
        case hgeSI_Position: // vector2
            position = *(hgeVector*)value;
            break;
        case hgeSI_Scale:    // vector2
            scale = *(hgeVector*)value;
            break;
        case hgeSI_Angle:    // float
            angle = *(float*)value;
            break;
        case hgeSI_Alpha:    // float
            alpha = *(float*)value;
            break;
        case hgeSI_Color:    // vector3
            color = *(Point3f*)value;
            break;
        case hgeSI_ClipRect: // Rect(short4)
            clipRect = *(Rect*)value;
            break;
        case hgeSI_AnchorOffPos: // vector2
            anchorOffPos = *(hgeVector*)value;
            break;
        default:
            assert(0);
            break;
        }
    }
    virtual const void *getState(StateId sid) const
    {
        switch(sid)
        {
        case hgeSI_Image:    // size_t
            return &image;
        case hgeSI_Position: // vector2
            return &position;
        case hgeSI_Scale:    // vector2
            return &scale;
        case hgeSI_Angle:    // float
            return &angle;
        case hgeSI_Alpha:    // float
            return &alpha;
        case hgeSI_Color:    // vector3
            return &color;
        case hgeSI_ClipRect: // Rect(short4)
            return &clipRect;
        case hgeSI_AnchorOffPos: // vector2
            return &anchorOffPos;
        default:
            assert(0);
            break;
        }
        return 0;
    }
    virtual void getState(StateId sid, void *value) const
    {
        switch(sid)
        {
        case hgeSI_Image:    // size_t
            *(Image*)value = image;
            break;
        case hgeSI_Position: // vector2
            *(hgeVector*)value = position;
            break;
        case hgeSI_Scale:    // vector2
            *(hgeVector*)value = scale;
            break;
        case hgeSI_Angle:    // float
            *(float*)value = angle;
            break;
        case hgeSI_Alpha:    // float
            *(float*)value = alpha;
            break;
        case hgeSI_Color:    // vector3
            *(Point3f*)value = color;
            break;
        case hgeSI_ClipRect: // Rect(short4)
            *(Rect*)value = clipRect;
            break;
        case hgeSI_AnchorOffPos: // vector2
            *(hgeVector*)value = anchorOffPos;
            break;
        default:
            assert(0);
            break;
        }
    }

    Image image;
    hgeVector position;
    hgeVector scale;
    float angle;
    float alpha;
    Point3f color;
    Rect clipRect;
    hgeVector anchorOffPos;
};

class hgeRenderer : public iRenderer
{
public:
    hgeRenderer()
    {
        assert(!hge);
        hge = hgeCreate(HGE_VERSION);
    }
    virtual ~hgeRenderer()
    {
        hge->Release();
    }
    /// @fn getName
    /// @brief get renderer's unique name
    const char *getName() const
    {
        return "HGE";
    }

    /// @fn getStateId
    /// @brief return id from name
    StateId getStateId(const char *stateName) const;
    /// @fn getStateName
    /// @brief return name from id
    const char *getStateName(StateId statusId) const;

    virtual void render(const iClipState &cs);
    iClipState *getClipState()
    {
        return &cs;
    }
protected:
    hgeClipState cs;
    static HGE *hge;
};

class hgeByteStream : public iByteStream
{
public:
    hgeByteStream() : data(0), size(0)
    {
        if (ByteStreamCount++ == 0)
        {
            hge = hgeCreate(HGE_VERSION);
        }
    }
    virtual ~hgeByteStream()
    {
        if (data)
        {
            hge->Resource_Free(data);
            size = 0;
        }
        if (--ByteStreamCount == 0)
        {
            hge->Release();
        }
    }
    virtual bool write(const void *buf, size_t len)
    {
        buf, len;
        assert(0 && "not allowed to write");
        return false;
    }
    virtual bool read(void *buf, size_t len)
    {
        if (len > size - cpos)
            return false;
        if (len > 0)
        {
            memcpy(buf, data + cpos, len);
            cpos += (DWORD)len;
        }
        return true;
    }
    /// seek to position
    virtual void seek(size_t pos)
    {
        cpos = min(DWORD(pos), size);
    }
    /// seek with an offset
    virtual void seekOffset(int offset)
    {
        seek(cpos + offset);
    }
    virtual size_t length() const
    {
        return size;
    }
    virtual size_t tell() const
    {
        return cpos;
    }
    void loadByteStream(const char *name)
    {
        if (data)
        {
            hge->Resource_Free(data);
            size = 0;
        }
        data = (char *)hge->Resource_Load(name, &size);
        cpos = 0;
    }

protected:
    char *data;
    DWORD size;
    DWORD cpos;
    static HGE *hge;
    static size_t ByteStreamCount;
};
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
        hgeByteStream *ByteStream = new hgeByteStream;
        ByteStream->loadByteStream(name);
        return ByteStream;
    }
    virtual iCurveDataSet *createCurveDataSet(const char *name);
    virtual iCurveInterpolater *createCurveInterpolater(const char *name);
    virtual void release(iByteStream *bs)
    {
        delete bs;
    }
    virtual void release(iCurveDataSet *cds)
    {
        delete cds;
    }
    virtual void release(iCurveInterpolater *ci)
    {
        delete ci;
    }
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

#endif
