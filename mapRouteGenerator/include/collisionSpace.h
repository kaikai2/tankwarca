#ifndef COMMON_COLLISION_SPACE_H
#define COMMON_COLLISION_SPACE_H

#include "hge.h"
#include "hgeVector.h"
#include <vector>
#include "caRect.h"
#include "caPoint2d.h"

#define USE_RESPLIT
using cAni::Rectf;
using cAni::Point2f;
using std::vector;

extern HGE *hge;
class CollisionEntity
{
};
class CollisionChecker
{
public:
    virtual bool checkCollision(const CollisionEntity &a, const CollisionEntity &b) = 0;
};
class Map : public CollisionChecker
{
public:
    void randomVertices();

    void render();
    vector<hgeVector> vertices;
    struct Line : public CollisionEntity
    {
        size_t va, vb;
        float len;
        bool operator < (const Line &o) const
        {
            return len < o.len;
        }
    };
    bool checkCollision(const CollisionEntity &a, const CollisionEntity &b)
    {
        return LineCross(*(Line *)&a, *(Line *)&b, vertices);
    }
    static float CrossProduct(const hgeVector &a, const hgeVector &b)
    {
        return a.y * b.x - a.x * b.y;
    }
    static bool LineCross(const Line &l1, const Line &l2, const vector<hgeVector> &vertices);
    vector<Line> lines;
};

class QuadSpace
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
    struct Node
    {
        Map::Line line;
        Rectf boundingBox;
    };
    const int d;
    QuadSpace(int depth, QuadSpace* _parent = 0) : parent(_parent), d(depth), child(0)
    {
        if (depth > 0)
        {
            child = (QuadSpace *)malloc(sizeof(QuadSpace) * NumSubSpaces);
            for (int i = 0; i < NumSubSpaces; i++)
                new(child + i) QuadSpace(depth - 1, this);
        }
    }
    virtual ~QuadSpace()
    {
        if (child)
        {
            for (int i = 0; i < NumSubSpaces; i++)
                child[i].~QuadSpace();
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
                child[LeftTop].setRange(r);
            }
            {
                Rectf r = range;
                r.rightBottom.x = center.x;
                r.leftTop.y = center.y;
                child[LeftBottom].setRange(r);
            }
            {
                Rectf r = range;
                r.leftTop.x = center.x;
                r.rightBottom.y = center.y;
                child[RightTop].setRange(r);
            }
            {
                Rectf r = range;
                r.leftTop = center;
                child[RightBottom].setRange(r);
            }
        }
        // update();
    }
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
    void addEntity(const Map::Line &line, const Rectf &box)
    {
        QuadSpace *qs = this;
        while(qs->child)
        {
            size_t cid = qs->getChildId(box);
            if (cid == size_t(-1))
                break;
            else
                qs = qs->child + cid;
        }
        Node n;
        n.line = line;
        n.boundingBox = box;
        qs->entities.push_back(n);
    }
    bool checkCollision(const Node &e, CollisionChecker &collisionChecker);

    void renderDebug(float alpha);

protected:
    QuadSpace *parent;
    QuadSpace *child;

    Rectf range;
    Point2f center;

    vector<Node> entities;
};
#endif // COMMON_COLLISION_SPACE_H
