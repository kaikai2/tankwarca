#ifndef _caHgePort_ClipState_H_
#define _caHgePort_ClipState_H_

#include "caHgePort_define.h"

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

#endif//_caHgePort_ClipState_H_

