#include "common/graphicEntity.h"
#include "game/dataCurvedani.h"
#include "gameDef.h"
#include "ca/caInterpolaters.h"
#include <sstream>

// nothing here
struct StateInfo
{
    StateId sid;
    const char *name;
};

template<typename key, typename value>
class BindMap
{
public:
    struct Info
    {
        key k;
        value v;
    };
    BindMap(Info *begin, Info *end, const key &ik, const value &iv)
        : invalidKey(ik), invalidValue(iv)
    {
        for (Info *i = begin; i != end; i++)
        {
            a2b[i->k] = i->v;
            b2a[i->v] = i->k;
        }
    }
    const value &find(const key &k) const
    {
        std::map<key, value>::const_iterator i = a2b.find(k);
        if (i == a2b.end())
        {
            return invalidValue;
        }
        return i->second;
    }
    const key &find(const value &v) const
    {
        std::map<value, key>::const_iterator i = b2a.find(v);
        if (i == b2a.end())
        {
            return invalidKey;
        }
        return i->second;
    }
private:
    BindMap &operator = (const BindMap &rhs)
    {
        rhs;
        return *this;
    }
    const key invalidKey;
    const value invalidValue;
    std::map<key, value> a2b;
    std::map<value, key> b2a;
};

// no way to use this
template<typename key>
class BindMap<key, key>
{
private:
    BindMap(){}
    ~BindMap(){}
};

typedef BindMap<cAni::StateId, std::string> BindMapState2Name;
static BindMapState2Name::Info s_StateInfos[] = 
{
    {dataSI_Position0,  "Position0" },    // vector2
    {dataSI_Position1,  "Position1" }, // vector2
    {dataSI_Position2,  "Position2" },    // vector2
    {dataSI_Alpha,      "Alpha" },    // float
    {dataSI_Color,      "Color" },    // vector3
};
static BindMapState2Name g_state2Name(s_StateInfos, s_StateInfos + sizeof(s_StateInfos) / sizeof(s_StateInfos[0]), cAni::StateId(cAni::InvalidStateId), "");


cAni::StateId DataTriangleRenderer::getStateId(const char *stateName) const
{
    return g_state2Name.find(stateName);
}

const char *DataTriangleRenderer::getStateName(cAni::StateId stateId) const
{
    return g_state2Name.find(stateId).c_str();
}

const char *DataTriangleRenderer::_getStateTypeName(StateId statusId) const
{
    switch(statusId)
    {
    case dataSI_Position0:
        return "Float2";
    case dataSI_Position1:
        return "Float2";
    case dataSI_Position2:
        return "Float2";
    case dataSI_Alpha:
        return "Float";
    case dataSI_Color:
        return "Float3";
    }
    return "";
}
typedef BindMap<cAni::InterpolateStyleId, std::string> BindMapInterpolateStyle2Name;
static BindMapInterpolateStyle2Name::Info s_InterpolateStyleInfos[] = 
{
    {dataII_Step,        "Step" },
    {dataII_Linear,      "Linear" },
    {dataII_Bspline,     "Bspline" },
};
static BindMapInterpolateStyle2Name g_InterpolateStyle2Name(s_InterpolateStyleInfos, s_InterpolateStyleInfos + sizeof(s_InterpolateStyleInfos) / sizeof(s_InterpolateStyleInfos[0]), cAni::InterpolateStyleId(cAni::InvalidInterpolateStyleId), "");

/// @fn getInterpolateStyleId
/// @brief return id from name
InterpolateStyleId DataTriangleRenderer::getInterpolateStyleId(const char *isName) const
{
    return g_InterpolateStyle2Name.find(isName);
}

/// @fn getInterpolateStyleName
/// @brief return name from id
const char *DataTriangleRenderer::getInterpolateStyleName(InterpolateStyleId isId) const
{
    return g_InterpolateStyle2Name.find(isId).c_str();
}

/// @fn getInterpolator
/// @brief get a working instance
iCurveInterpolater *DataTriangleRenderer::getInterpolator(StateId stateId, InterpolateStyleId isId) const
{
    if (mapInterpolaters[std::make_pair(stateId, isId)] == 0)
    {
        std::stringstream ss;
        ss << this->getInterpolateStyleName(isId) << this->_getStateTypeName(stateId);

        iCurveInterpolater *createCurveInterpolater(const char *name);
        mapInterpolaters[std::make_pair(stateId, isId)] = createCurveInterpolater(ss.str().c_str());
    }
    return mapInterpolaters[std::make_pair(stateId, isId)];
}

DataTriangleRenderer::DataTriangleRenderer()
{

}

DataTriangleRenderer::~DataTriangleRenderer()
{

}
void DataTriangleRenderer::render(const Point2f &pos)
{
    for (size_t i = 0; i < tripleList.size(); i++)
        iRenderQueue::getSingleton().render(pos, tripleList[i], GLI_UI, true);

    tripleList.clear();
}
template<template <typename> class T >
inline iCurveInterpolater *createInterpolator_all(const char *name)
{
    if (0 == strcmp("Float", name))
    {
        return new T<float>;
    }
    else if (0 == strcmp("Float2", name))
    {
        return new T<Point2f>;
    }
    else if (0 == strcmp("Float3", name))
    {
        return new T<Point3f>;
    }
    else if (0 == strcmp("Rect", name))
    {
        return new T<Rect>;
    }
    /// and you could add any data types you want here
    //else if if (0 == strncmp("MyDataType", name, strlen("MyDataType")))
    //{
    //  return new T<MyDataType>;
    //}
    return 0;
}

template<template <typename> class T >
inline iCurveInterpolater *createInterpolator(const char *name)
{
    return createInterpolator_all<T>(name);
}

template<>
inline iCurveInterpolater *createInterpolator<StepInterpolater>(const char *name)
{
    if (0 == strcmp("Image", name)) // Image÷ª÷ß≥÷StepInterpolater
    {
        return new StepInterpolater<Image>;
    }
    return createInterpolator_all<StepInterpolater>(name);
}

iCurveInterpolater *createCurveInterpolater(const char *name)
{
    if (0 == strncmp("Step", name, 4))
    {
        return createInterpolator<StepInterpolater>(name + 4);
    }
    else if (0 == strncmp("Linear", name, 6))
    {
        return createInterpolator<LinearInterpolater>(name + 6);
    }
    else if (0 == strncmp("Bspline", name, 7))
    {
        return createInterpolator<BsplineInterpolater>(name + 7);
    }
    /// and you could add any interpolaters you want here, such as sin/cos/circle...
    //else if if (0 == strncmp("sin", name, 3))
    //{
    //  return createInterpolator<SinInterpolater>(name + 3);
    //}
    assert(0 && "unsupported curve interpolater.");
    return 0;
}
