#ifndef COMMON_QUAD_SPACE_H
#define COMMON_QUAD_SPACE_H

#include <vector>
#include "caPoint2d.h"
#include "common/entity.h"
#include "common/collisionEntity.h"
#include "common/graphicEntity.h"
#include "common/utils.h"

using std::vector;
using cAni::Point2f;

class QuadSpace : public NoCopy
{
public:
    enum
    {
        RightBit = 1,
        BottomBit = 2,

        LeftTop = 0,
        RightTop,
        LeftBottom,
        RightBottom,
        NumSubSpaces = 4,
    };
    const int d;
    QuadSpace(int depth, QuadSpace* _parent = 0) : parent(_parent), d(depth), child(0)
    {
    }
    virtual ~QuadSpace()
    {
        if (child)
        {
            free(child);
            child = 0;
        }
    }
    void setRange(const Rectf &_range)
    {
        range = _range;

        this->center = (range.leftTop + range.rightBottom) / 2;
        if (child)
        {
            {
                Rectf r = range;
                r.rightBottom = center;
                getChild(LeftTop).setRange(r);
            }
            {
                Rectf r = range;
                r.rightBottom.x = center.x;
                r.leftTop.y = center.y;
                getChild(LeftBottom).setRange(r);
            }
            {
                Rectf r = range;
                r.leftTop.x = center.x;
                r.rightBottom.y = center.y;
                getChild(RightTop).setRange(r);
            }
            {
                Rectf r = range;
                r.leftTop = center;
                getChild(RightBottom).setRange(r);
            }
        }
        // update();
    }
    const Rectf &getRange() const
    {
        return range;
    }
    virtual QuadSpace &getChild(size_t id) = 0;
    virtual const QuadSpace &getChild(size_t id) const = 0;
    virtual QuadSpace &getParent() = 0;
    virtual const QuadSpace &getParent() const = 0;

    size_t getChildId(const Rectf &r) const
    {
        size_t bitId = 0;
        if (r.leftTop.x > center.x)
            bitId |= RightBit;
        else if (r.rightBottom.x > center.x)
            return size_t(-1);

        if (r.leftTop.y > center.y)
            bitId |= BottomBit;
        else if (r.rightBottom.y > center.y)
            return size_t(-1);

        return bitId;
    }
    // 是否有交集
    bool isIntersected(const Rectf &r) const
    {
        return r.leftTop.x <= this->range.rightBottom.x && r.rightBottom.x >= this->range.leftTop.x &&
            r.leftTop.y <= this->range.rightBottom.y && r.rightBottom.y >= this->range.leftTop.y;
    }
    // 是否包含
    bool isContained(const Rectf &r) const
    {
        return r.leftTop.x >= this->range.leftTop.x && r.rightBottom.x <= this->range.rightBottom.x && 
            r.leftTop.y >= this->range.leftTop.y && r.rightBottom.y <= this->range.rightBottom.y;
    }

    // 获得元素的包围盒
    // virtual Rectf& getBoundingBox(void *entity) const = 0;
protected:

    void *allocBuffer(size_t sizeofAnObject);

    QuadSpace *parent;
    QuadSpace *child;

    Rectf range;
    Point2f center;
};
#endif // COMMON_QUAD_SPACE_H
