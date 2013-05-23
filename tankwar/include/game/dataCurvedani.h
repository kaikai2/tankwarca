#ifndef GAME_DATA_CURVEDANI_H
#define GAME_DATA_CURVEDANI_H

#include "hgeCurvedAni.h"
#include "caPoint2d.h"
#include "common/utils.h"
#include <vector>
#include <map>
using cAni::Point2f;

enum dataCurvedAniStateId
{
    dataSI_Position0, // vector2
    dataSI_Position1, // vector2
    dataSI_Position2, // vector2
    dataSI_Alpha,    // float
    dataSI_Color,    // vector3
};

enum dataInterpolatorStateId
{
    dataII_Step,
    dataII_Linear,
    dataII_Bspline,
};

/// 实现下列接口
/// 参数
class dataClipState : public iClipState
{
public:
    dataClipState() : alpha(1), color(1, 1, 1)
    {
        position[0] = hgeVector(0, 0);
        position[1] = hgeVector(0, 0);
        position[2] = hgeVector(0, 0);
    }
    dataClipState(const dataClipState &rhs) : alpha(1), color(1, 1, 1)
    {
        reset(&rhs);
    }
    virtual void reset(const iClipState *defaultClipState = 0)
    {
        if (defaultClipState)
        {
            defaultClipState->getState(dataSI_Position0, position[0]);
            defaultClipState->getState(dataSI_Position1, position[1]);
            defaultClipState->getState(dataSI_Position2, position[2]);
            defaultClipState->getState(dataSI_Alpha,     alpha);
            defaultClipState->getState(dataSI_Color,     color);
        }
        else
        {
            position[0] = hgeVector(0, 0);
            position[1] = hgeVector(0, 0);
            position[2] = hgeVector(0, 0);
            alpha = 1;
            color = Point3f(1, 1, 1);
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
        case dataSI_Position0: // vector2
            return position;
        case dataSI_Position1: // vector2
            return position + 1;
        case dataSI_Position2: // vector2
            return position + 2;
        case dataSI_Alpha:    // float
            return &alpha;
        case dataSI_Color:    // vector3
            return &color;
        default:
            assert(0);
            break;
        }
        return 0;
    }
    virtual iClipState *clone() const
    {
        return new dataClipState(*this);
    }
    virtual void release()
    {
        delete this;
    }

    // merge style: 父子结点融合方式.
    virtual void setStateMergeStyle(StateId sid, MergeStyleId mergeStyle)
    {
        sid, mergeStyle;
    }
    virtual bool makeSample(StateId sid, iCurveInterpolater &rInterpolater, iCurveDataSet &rDataSet, int time)
    {
        void *pData = _getState(sid);
        return pData != 0 && rInterpolater.getSample(sid, rDataSet, time, pData);
    }
    hgeVector position[3];
    float alpha;
    Point3f color;

    MergeStyleId mergeStyle[NumOfHgeSI];
};

class DataTriangleRenderer : public iRenderer
{
public:
    DataTriangleRenderer();
    virtual ~DataTriangleRenderer();

    /// @fn getName
    /// @brief get renderer's unique name
    const char *getName() const
    {
        return "DataTriangle";
    };
    /// @fn getStateId
    /// @brief return id from name
    StateId getStateId(const char *stateName) const;
    /// @fn getStateName
    /// @brief return name from id
    const char *getStateName(StateId stateId) const;

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

	virtual iCurveDataSet *createCurveDataSet(const char *name)
	{
		if (0 == strcmp("Float", name))
		{
			return new hgeCurveDataSet<float>;
		}
		else if (0 == strcmp("Float2", name))
		{
			return new hgeCurveDataSet<hgeVector>;
		}
		else if (0 == strcmp("Float3", name))
		{
			return new hgeCurveDataSet<Point3f>;
		}
		else if (0 == strcmp("Rect", name))
		{
			return new hgeCurveDataSet<Rect>;
		}
	}
	virtual void releaseCurveDataSet(iCurveDataSet *cds)
	{
		delete cds;
	}
    virtual void render(const iClipState &rClipState, const iClipState &rParentClipState)
    {
        rParentClipState;

        hgeTriple t;
        for (int i = 0; i < 3; i++)
        {
            const hgeVector *p = (const hgeVector *)rClipState.getState(dataSI_Position0 + i);
            t.v[i].x = p->x;
            t.v[i].y = p->y;
            t.v[i].z = 0;
            float alpha;
            rClipState.getState(dataSI_Alpha, alpha);
            Point3f color;
            rClipState.getState(dataSI_Color, color);

            int a = clamp(int(alpha * 255), 0, 255);
            int r = clamp(int(color.x * 255), 0, 255);
            int g = clamp(int(color.y * 255), 0, 255);
            int b = clamp(int(color.z * 255), 0, 255);

            t.v[i].col = ARGB(a, r, g, b);
            t.v[i].tx = t.v[i].ty = 0;
        }
        t.tex = 0;
        t.blend = BLEND_DEFAULT;
        tripleList.push_back(t);
    }
    void render(const Point2f &pos);
protected:
	const char *_getStateTypeName(StateId statusId) const;
    std::vector<hgeTriple> tripleList;
    dataClipState cs;
    mutable std::map<std::pair<StateId, InterpolateStyleId>, iCurveInterpolater *> mapInterpolaters;
};

#endif
