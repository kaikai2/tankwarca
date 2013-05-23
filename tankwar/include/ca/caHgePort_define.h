#ifndef _CaHgePort_Define_H_
#define _CaHgePort_Define_H_

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

#endif//_CaHgePort_Define_H_
