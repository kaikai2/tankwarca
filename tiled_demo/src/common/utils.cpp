#include <algorithm>

#include "caRect.h"

#include "common/utils.h"

using std::swap;
using cAni::Point2f;
using cAni::Rectf;

// The intersection of two lines is a solution to P0+s0*D0 = P1+s1*D1.
// Rewrite this as s0*D0 - s1*D1 = P1 - P0 = Q.  If D0.Dot(Perp(D1)) = 0,
// the lines are parallel.  Additionally, if Q.Dot(Perp(D1)) = 0, the
// lines are the same.  If D0.Dot(Perp(D1)) is not zero, then
//   s0 = Q.Dot(Perp(D1))/D0.Dot(Perp(D1))
// produces the point of intersection.  Also,
//   s1 = Q.Dot(Perp(D0))/D0.Dot(Perp(D1))
bool LineCrossPoint(const Point2f &va, const Point2f &vb, const Point2f &vc, const Point2f &vd, Point2f &out)
{
    const Point2f D0 = vb - va;
    const Point2f D1 = vd - vc;
    const Point2f Q = vc - va;
    float D0dD1 = D0 ^ D1;
    if (D0dD1 == 0)
    {
        // 平行或者重叠
        return false;
    }
    float QdD1 = Q ^ D1;
    float s0 = QdD1 / D0dD1;
    if (s0 <= 0 || s0 >= 1)
        return false;
    float QdD0 = Q ^ D0;
    float s1 = QdD0 / D0dD1;
    if (s1 <= 0 || s1 >= 1)
        return false;
    out = va + D0 * s0;
    return true;
}

bool LineCross(const Point2f &va, const Point2f &vb, const Point2f &vc, const Point2f &vd)
{
    Rectf b1(va.x, vb.x, va.y, vb.y);
    Rectf b2(vc.x, vd.x, vc.y, vd.y);
    if (b1.leftTop.x > b1.rightBottom.x)
        swap(b1.leftTop.x, b1.rightBottom.x);
    if (b1.leftTop.y > b1.rightBottom.y)
        swap(b1.leftTop.y, b1.rightBottom.y);
    if (b2.leftTop.x > b2.rightBottom.x)
        swap(b2.leftTop.x, b2.rightBottom.x);
    if (b2.leftTop.y > b2.rightBottom.y)
        swap(b2.leftTop.y, b2.rightBottom.y);
    if (b1.leftTop.x > b2.rightBottom.x || b1.rightBottom.x < b2.leftTop.x || 
        b1.leftTop.y > b2.rightBottom.y || b1.rightBottom.y < b2.leftTop.y)
        return false;

    Point2f dac = vc - va;
    Point2f dab = vb - va;
    Point2f dad = vd - va;
    Point2f dcb = vb - vc;
    Point2f dcd = vd - vc;
    Point2f dca = va - vc;

    return (dac ^ dab) * (dab ^ dad) > 0 && 
        (dcb ^ dcd) * (dcd ^ dca) > 0;
}

float DistancePointToEdge(const Point2f &p, const Point2f &a, const Point2f &b)
{
    Point2f kDiff = p - a;
    Point2f direction = (b - a).Normalize();
    float len = (b - a).Length();
    float fParam = direction * kDiff;
    Point2f closestPoint;
    if (fParam > 0)
    {
        if (fParam < len)
        {
            closestPoint = a + direction * fParam;
        }
        else
        {
            closestPoint = b;
        }
    }
    else
    {
        closestPoint = a;
    }

    kDiff = closestPoint - p;
    return kDiff.Length();
}
