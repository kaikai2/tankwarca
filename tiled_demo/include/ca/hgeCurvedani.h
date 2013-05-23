#ifndef HGE_CURVEDANI_H
#define HGE_CURVEDANI_H

#include "hge.h"
#include "curvedani.h"
#include "caImage.h"
#include "hgevector.h"
#include "caPoint3d.h"
#include "caRect.h"
#include "Wm4Matrix3.h"

/// hge版本且用一下stl,但在curvedani内当不该出现stl，以免非c++用户怪罪
#include <vector>
#include <map>
#include <string>

using cAni::StateId;
using cAni::InterpolateStyleId;
using cAni::MergeStyleId;
using cAni::iTexture;
using cAni::iCurveDataSet;
using cAni::iCurveInterpolater;
using cAni::iStateMergerIf;
using cAni::iStateMergerComposition;
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
    //hgeSI_Transform,    // Wm4::matrix3

    NumOfHgeSI,
};

enum hgeCurvedAniMergeStyleId
{
	hgeMI_None,
    hgeMI_Gain,
    hgeMI_Reduce,
	hgeMI_SaturateColor, // saturate in [0,1]

    NumOfHgeMI,
};

enum hgeCurvedAniInterpolateStyleId
{
    hgeII_Step,
    hgeII_Linear,
    hgeII_Bspline,

    NumOfHgeII,
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
        if (tex)
        {
            return size_t(hge->Texture_GetWidth(tex));
        }
        return 0;
    }
    virtual size_t getHeight() const
    {
        if (tex)
        {
            return size_t(hge->Texture_GetHeight(tex));
        }
        return 0;
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
	std::vector<DataNode> data;
};

/// 参数
class hgeClipState : public iClipState
{
public:
    hgeClipState() : image(), position(0, 0), scale(1, 1), angle(0), alpha(1), color(1, 1, 1), clipRect(), anchorOffPos(0, 0)
    {
    }
    hgeClipState(const hgeClipState &rhs) : image(), position(0, 0), scale(1, 1), angle(0), alpha(1), color(1, 1, 1), clipRect(), anchorOffPos(0, 0)
    {
        reset(&rhs);
    }
    virtual void reset(const iClipState *defaultClipState = 0)
    {
        if (defaultClipState)
        {
            hgeClipState &rOther = *(hgeClipState *)defaultClipState;
            defaultClipState->getState(hgeSI_Image,    image);
            defaultClipState->getState(hgeSI_Position, position);
            defaultClipState->getState(hgeSI_Scale,    scale);
            defaultClipState->getState(hgeSI_Angle,    angle);
            defaultClipState->getState(hgeSI_Alpha,    alpha);
            defaultClipState->getState(hgeSI_Color,    color);
            defaultClipState->getState(hgeSI_ClipRect, clipRect);
            defaultClipState->getState(hgeSI_AnchorOffPos, anchorOffPos);
            //defaultClipState->getState(hgeSI_Transform, transform);

            for (int i = 0; i < NumOfHgeSI; i++)
            {
                setStateMergeStyle((StateId)i, rOther._getStateMergeStyle((StateId)i));
            }
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
            memset(mergeStyle, 0, sizeof(mergeStyle));
        }
    }
    //virtual StateId StringToStateId(const char *name)
    //{
    //    return name, 0;
    //}
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
        //case hgeSI_Transform:// Wm4::Matrix3f
        //    return &transform;
        default:
            assert(0);
            break;
        }
        return 0;
    }
    // merge style: 父子结点融合方式.
private:
    MergeStyleId _getStateMergeStyle(StateId sid) const
    {
        if (sid >= 0 && sid < NumOfHgeSI)
        {
            return mergeStyle[sid];
        }
        return 0;
    }
public:
    virtual void setStateMergeStyle(StateId sid, MergeStyleId mergeStyle)
    {
        if (sid >= 0 && sid < NumOfHgeSI)
        {
            this->mergeStyle[sid] = mergeStyle;
        }
    }

    virtual iClipState *clone() const
    {
        return new hgeClipState(*this);
    }
    virtual void release()
    {
        delete this;
    }

    virtual bool makeSample(StateId sid, iCurveInterpolater &rInterpolater, iCurveDataSet &rDataSet, int time)
    {
        void *pData = _getState(sid);
        return pData != 0 && rInterpolater.getSample(sid, rDataSet, time, pData);
    }
    
	void merge(StateId sid, iStateMergerComposition &rStateMergerComposition, const hgeClipState &rChild, const hgeClipState &rParent)
	{
        static bool bNeedMerge[] = 
        {
            false, //hgeSI_Image,    // size_t
            false, //hgeSI_Position, // vector2
            false, //hgeSI_Scale,    // vector2
            false, //hgeSI_Angle,    // float
            true,//hgeSI_Alpha,    // float
            true,//hgeSI_Color,    // vector3
            true,//hgeSI_ClipRect, // Rect(short4)
            false,//hgeSI_AnchorOffPos, // vector2
            //hgeSI_Transform,    // Wm4::matrix3
        };
        if (sid < NumOfHgeSI && bNeedMerge[sid])
        {
            rStateMergerComposition.setMergerStyle(sid, rChild._getStateMergeStyle(sid));
            rStateMergerComposition.merge(sid, rChild.getState(sid), rParent.getState(sid), this->_getState(sid));
        }
	}

    void getMatrix(Wm4::Matrix3f &mResult, MergeStyleId msAnchorOffPos, MergeStyleId msAngle, MergeStyleId msScale, MergeStyleId msPosition) const
    {
        mResult.MakeIdentity();

        if (msAnchorOffPos != 0)
        {
            Wm4::Matrix3f m33(
                1, 0, 0,
                0, 1, 0,
                -anchorOffPos.x, -anchorOffPos.y, 1);
            mResult = mResult * m33;
        }
        if (msAngle != 0)
        {
            Wm4::Matrix3f m33_rot;
            m33_rot.FromAxisAngle(Wm4::Vector3f(0, 0, 1), angle);
            mResult = mResult * m33_rot;
        }
        if (msScale != 0)
        {
            Wm4::Matrix3f mScale;
            mScale.MakeDiagonal(scale.x, scale.y, 1);
            mResult = mResult * mScale;
        }
        if (msPosition != 0)
        {
            //if (msAnchorOffPos != 0)
            //{
            //    Wm4::Matrix3f mPos(
            //        1, 0, 0,
            //        0, 1, 0,
            //        position.x + anchorOffPos.x, position.y + anchorOffPos.y, 1);
            //    mResult = mResult * mPos;
            //}
            //else
            {
                Wm4::Matrix3f mPos(
                    1, 0, 0,
                    0, 1, 0,
                    position.x, position.y, 1);
                mResult = mResult * mPos;
            }
        }
        else
        {
            //if (msAnchorOffPos != 0)
            //{
            //    Wm4::Matrix3f m33(
            //        1, 0, 0,
            //        0, 1, 0,
            //        anchorOffPos.x, anchorOffPos.y, 1);
            //    mResult = mResult * m33;
            //}
        }
    }
	static void updateMatrix(Wm4::Matrix3f &mResult, const hgeClipState &rChild, const hgeClipState &rParent)
	{
        Wm4::Matrix3f mChild;
        rChild.getMatrix(mChild, 1, 1, 1, 1);

        Wm4::Matrix3f mParent;
        rParent.getMatrix(mParent,
            rChild._getStateMergeStyle(hgeSI_AnchorOffPos),
            rChild._getStateMergeStyle(hgeSI_Angle),
            rChild._getStateMergeStyle(hgeSI_Scale),
            rChild._getStateMergeStyle(hgeSI_Position));

        mResult = mChild * mParent;
	}

    Image image;
    hgeVector position;
    hgeVector scale;
    float angle;
    float alpha;
    Point3f color;
    Rect clipRect;
    hgeVector anchorOffPos;
    //Wm4::Matrix3f transform;

    MergeStyleId mergeStyle[NumOfHgeSI];
};

class hgeStateMergerComposition : public iStateMergerComposition
{
public:
	hgeStateMergerComposition();
	virtual ~hgeStateMergerComposition();

	// 更换和设置融合方式
	virtual void setMergerStyle(cAni::StateId sid, cAni::MergeStyleId mid);
protected:
	// 容器类元素访问接口
	virtual iStateMergerIf *getMerger(cAni::StateId sid) const;
private:
	iStateMergerIf *mergers[NumOfHgeSI];
};

class hgeRenderer : public iRenderer
{
public:
    hgeRenderer();
    virtual ~hgeRenderer();

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

    /// @fn getInterpolateStyleId
    /// @brief return id from name
    virtual InterpolateStyleId getInterpolateStyleId(const char *isName) const;

    /// @fn getInterpolateStyleName
    /// @brief return name from id
    virtual const char *getInterpolateStyleName(InterpolateStyleId isId) const;

    /// @fn getClipState
    /// @brief get a working instance
    virtual iClipState *getClipState()
    {
        return &cs;
    }
    /// @fn getInterpolator
    /// @brief get a working instance
    virtual iCurveInterpolater *getInterpolator(StateId stateId, InterpolateStyleId isId) const;

    virtual void render(const iClipState &rClipState, const iClipState &rParentClipState);
protected:
    const char *_getStateTypeName(StateId statusId) const;

    hgeClipState cs;
    static HGE *hge;
    mutable std::map<std::pair<StateId, InterpolateStyleId>, iCurveInterpolater *> mapInterpolaters;
	//std::map<MergeStyleId, iStateMergerIf*> mapMergers;
};

class hgeByteStream : public iByteStream
{
public:
    hgeByteStream() : data(0), size(0)
    {
        if (byteStreamCount++ == 0)
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
        if (--byteStreamCount == 0)
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
    static size_t byteStreamCount;
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
        hgeByteStream *byteStream = new hgeByteStream;
        byteStream->loadByteStream(name);
        return byteStream;
    }
    virtual iCurveDataSet *createCurveDataSet(const char *name);
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

#endif
