#ifndef GAME_ROUTEMAP_H
#define GAME_ROUTEMAP_H

#include <cassert>
#include <vector>
#include <list>
#include <float.h>

#include "common/quadSpace.h"
#include "common/utils.h"

using std::vector;
using std::list;
using cAni::Point2f;

class RouteMap;
class RouteMapObjSpace;

enum DataType
{
    DT_Edge          = 1<<0,
    DT_Poly          = 1<<1,
    DT_RouteTriangle = 1<<2,
    DT_BlockTriangle = 1<<3,
    DT_Triangle      = DT_RouteTriangle | DT_BlockTriangle,
    DT_PolyEdge      = 1<<4,
};

class MapObjCuller : public NoCopy
{
public:
    MapObjCuller(vector<size_t> &_objs) : objects(_objs)
    {
    }
    //void cull(RouteMapObjSpace &space);
    bool filter(DataType dt) const
    {
        return (dt & dataType) != 0;
    }

    DataType dataType;
    Rectf viewer;
    vector<size_t> &objects;
};
class MapObjCuller;
class DistanceCompare;
class RouteMapObjSpace : public QuadSpace
{
public:
    struct Node
    {
        DataType dataType;
        size_t dataId;
        Rectf boundingBox;
    };
    RouteMapObjSpace(int depth, RouteMapObjSpace* _parent = 0) : QuadSpace(depth, _parent)
    {
        if (depth > 0)
        {
            child = (RouteMapObjSpace *)allocBuffer(sizeof(RouteMapObjSpace));
            for (int i = 0; i < NumSubSpaces; i++)
                new(&getChild(i)) RouteMapObjSpace(depth - 1, this);
        }
    }
    virtual ~RouteMapObjSpace()
    {
        if (child)
        {
            for (int i = 0; i < NumSubSpaces; i++)
                getChild(i).~RouteMapObjSpace();
        }
    }
    RouteMapObjSpace &getChild(size_t id)
    {
        return ((RouteMapObjSpace*)child)[id];
    }
    const RouteMapObjSpace &getChild(size_t id) const
    {
        return ((const RouteMapObjSpace*)child)[id];
    }
    RouteMapObjSpace &getParent()
    {
        return *(RouteMapObjSpace*)parent;
    }
    const RouteMapObjSpace &getParent() const
    {
        return *(const RouteMapObjSpace*)parent;
    }

    void addEntity(size_t dataId, DataType dataType, const Rectf &box);
    size_t getClosestEntity(const Point2f &point, float &minDistance, unsigned short filter) const;
    size_t getClosestEntity(const Point2f &point, DistanceCompare &cmp) const;
    bool checkCollision(const Node &e, RouteMap &rmap);

    void renderDebug(float alpha);
    void clear()
    {
        entities.clear();
        if (child)
        {
            for (size_t i = 0; i < NumSubSpaces; i++)
                getChild(i).clear();
        }
    }
    void cull(MapObjCuller &culler) const;
protected:

    vector<Node> entities;
};

class RouteMap
{
public:
    struct Polygon
    {
        vector<size_t> vertices;
    };
    struct Edge
    {
        size_t va, vb;
        float len;
        size_t t0, t1; // side triangles
        bool bBad;
        bool failedToTurn;
        Edge() : bBad(false), failedToTurn(false)
        {
        }
        size_t getOtherTriangle(size_t t) const
        {
            assert(t == t0 || t == t1);
            return t == t0 ? t1 : t0;
        }
        bool operator < (const Edge &o) const
        {
            return len < o.len;
        }
    };
    struct Triangle
    {
        enum
        {
            PointA = 0,
            PointB,
            PointC,

            EdgeBC = 0,
            EdgeAC,
            EdgeAB,
        };
        Triangle() : block(0)
        {
            v[0] = v[1] = v[2] = size_t(-1);
            t[0] = t[1] = t[2] = size_t(-1);
            e[0] = e[1] = e[2] = size_t(-1);
        }
        bool hasVertex(size_t _v) const
        {
            return v[PointA] == _v || v[PointB] == _v || v[PointC] == _v;
        }
        size_t v[3];    // vertices' index
        size_t t[3];
        size_t e[3];
        char block;
        //size_t tab, tbc, tac; // neighbor triangles' index
    };
    RouteMap() : space(5), polySpace(5)
    {
    }
    void BeginAddLine();
    void addLine(const Point2f &a, const Point2f &b);
    void EndAddLine();

    inline bool checkCollision(const size_t lineId_a, const size_t lineId_b) const;
    inline bool LineCrossPoint(const Edge &a, const Edge &b, Point2f &out) const;
    inline float DistancePointToEdge(size_t pointId, const Edge &e) const;

    void setRange(const Rectf &range);
    void randomVertices(const Rectf &range);
    //void generateMapFromVertices_old();
    void generateMapFromVertices();

    void renderDebug() const;

    bool aStarSearch(vector<size_t> &result, size_t from, size_t to) const;
    void FindPath(list<Point2f> &result, const Point2f &from, const Point2f &to) const;
    
    void addPolygon(const vector<Point2f> &polygonVertices);
    bool insidePolygon(const Point2f &p, const Polygon &polygon) const;

    bool turnEdge(Edge &e);

    vector<Point2f> verticesBuffer;
    vector<Triangle> triangles;
    vector<Edge> edges;
    vector<Polygon> polygons;
    Rectf range;
    RouteMapObjSpace space;
    RouteMapObjSpace polySpace;
    Point2f getCenter(const Triangle &t) const;
    Rectf getBoundingBox(const Edge &e) const;
    Rectf getBoundingBox(const Triangle &t) const;
    Rectf getBoundingBox(const Polygon &p) const;

    size_t numOriginalVertices;
    vector<Edge> polyEdges;
};

inline
bool RouteMap::checkCollision(const size_t lineId_a, const size_t lineId_b) const
{
    assert(lineId_a < edges.size());
    assert(lineId_b < edges.size());
    const Edge &la = edges[lineId_a];
    const Edge &lb = edges[lineId_b];
    return ::LineCross(verticesBuffer[la.va], verticesBuffer[la.vb],
        verticesBuffer[lb.va], verticesBuffer[lb.vb]);
}

inline
bool RouteMap::LineCrossPoint(const Edge &a, const Edge &b, Point2f &out) const
{
    assert(a.va < verticesBuffer.size());
    assert(a.vb < verticesBuffer.size());
    assert(b.va < verticesBuffer.size());
    assert(b.vb < verticesBuffer.size());
    return ::LineCrossPoint(verticesBuffer[a.va], verticesBuffer[a.vb],
        verticesBuffer[b.va], verticesBuffer[b.vb], out);
}

inline
float RouteMap::DistancePointToEdge(size_t pointId, const Edge &e) const
{
    assert(pointId < verticesBuffer.size());
    assert(e.va < verticesBuffer.size());
    assert(e.vb < verticesBuffer.size());
    return ::DistancePointToEdge(verticesBuffer[pointId], verticesBuffer[e.vb],
        verticesBuffer[e.va]);
}

#endif//GAME_ROUTEMAP_H
