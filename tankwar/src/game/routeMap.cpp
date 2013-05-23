#pragma warning(disable: 4996)

#include <string>
#include <cassert>
#include <vector>
#include <map>
#include <algorithm>

#include "map.h"
#include "mapObj.h"
#include "staticMapObj.h"
#include "common/entityManager.h"
#include "common/graphicEntity.h"
#include "delaunay.h"

using std::vector;
using std::map;
using std::find;
using std::swap;
using std::sort;

void RouteMapObjSpace::addEntity(size_t dataId, DataType dataType, const Rectf &box)
{
    RouteMapObjSpace *qs = this;
    while(qs->child)
    {
        size_t cid = qs->getChildId(box);
        if (cid == size_t(-1))
            break;
        else
            qs = &qs->getChild(cid);
    }
    Node n;
    n.dataType = dataType;
    n.dataId = dataId;
    n.boundingBox = box;
    qs->entities.push_back(n);
}

size_t RouteMapObjSpace::getClosestEntity(const Point2f &point, float &minDistance, unsigned short filter) const
{
    size_t entityId = size_t(-1);
    minDistance = FLT_MAX;
    for (size_t i = 0; i < entities.size(); i++)
    {
        if ((filter & entities[i].dataType) == 0)
            continue;

        float distance = entities[i].boundingBox.distance(point);
        if (distance < minDistance)
        {
            minDistance = distance;

            entityId = entities[i].dataId;
        }
    }
    if (child)
    {
        for (size_t i = 0; i < NumSubSpaces; i++)
        {
            float distance = getChild(i).getRange().distance(point);
            if (distance < minDistance)
            {
                size_t eid = getChild(i).getClosestEntity(point, distance, filter);
                if (distance < minDistance)
                {
                    minDistance = distance;
                    entityId = eid;
                }
            }
        }
    }
    return entityId;
}

class DistanceCompare
{
public:
    DistanceCompare() : minDistance(FLT_MAX)
    {
    }
    
    virtual bool filter(DataType dt) const = 0;
    virtual bool operator () (const Point2f &point, size_t id) = 0;
    float getMinDistance() const
    {
        return minDistance;
    }
    void resetDistance()
    {
        minDistance = FLT_MAX;
    }
protected:
    float minDistance;
};

size_t RouteMapObjSpace::getClosestEntity(const Point2f &point, DistanceCompare &cmp) const
{
    size_t entityId = size_t(-1);
    for (size_t i = 0; i < entities.size(); i++)
    {
        if (!cmp.filter(entities[i].dataType))
            continue;

        float distance = entities[i].boundingBox.distance(point);
        if (distance < cmp.getMinDistance() && cmp(point, entities[i].dataId))
        {
            entityId = entities[i].dataId;
        }
    }
    if (child)
    {
        for (size_t i = 0; i < NumSubSpaces; i++)
        {
            float distance = getChild(i).getRange().distance(point);
            if (distance < cmp.getMinDistance())
            {
                size_t eid = getChild(i).getClosestEntity(point, cmp);
                if (eid != size_t(-1))
                    entityId = eid;
            }
        }
    }
    return entityId;
}

class RouteMapTriangleDistanceCompare : public DistanceCompare
{
public:
    RouteMapTriangleDistanceCompare(const RouteMap &_rmap) : rmap(_rmap)
    {
    }
    virtual bool filter(DataType dt) const
    {
        return (filterDataType & dt) != 0;
    }
    virtual bool operator () (const Point2f &point, size_t id)
    {
        assert(id < rmap.triangles.size());
        const Point2f &center = rmap.getCenter(rmap.triangles[id]);
        float len = (center - point).Length();
        if (len < minDistance)
        {
            minDistance = len;
            return true;
        }
        return false;
    }
    unsigned short filterDataType;
protected:
    const RouteMap &rmap;
};

void RouteMap::renderDebug() const
{
    float alpha = 128;
    vector<size_t> objects;
    MapObjCuller culler(objects);
    culler.viewer = iRenderQueue::getSingleton().getWindowViewer();
    culler.dataType = DT_Triangle;
    space.cull(culler);

#define ROUTEMAP_RENDERDEBUG_INTERSECT_POINTS       0
#define ROUTEMAP_RENDERDEBUG_TRIANGLE_EDGES         1
#define ROUTEMAP_RENDERDEBUG_BLOCK_TRIANGLE_EDGES   1
#define ROUTEMAP_RENDERDEBUG_FILL_BLOCK_AREA        1

    for (size_t i = 0; i < objects.size(); i++)
    {
        size_t tid = objects[i];
        assert(tid < triangles.size());
        const Triangle &tri = triangles[tid];
        const Point2f center = getCenter(tri);
        //if (!tri.block)

        const Point2f &pa = verticesBuffer[tri.v[Triangle::PointA]];
        const Point2f &pb = verticesBuffer[tri.v[Triangle::PointB]];
        const Point2f &pc = verticesBuffer[tri.v[Triangle::PointC]];

#if ROUTEMAP_RENDERDEBUG_INTERSECT_POINTS
        const Point2f &da = (pa - center).Normalize();
        const Point2f &db = (pb - center).Normalize();
        const Point2f &dc = (pc - center).Normalize();
        if (tri.va >= numOriginalVertices)
        {
            iRenderQueue::getSingleton().render(pa - Point2f(3, 0), pa + Point2f(2, 0), ARGB(255, 0, 0, 255));
            iRenderQueue::getSingleton().render(pa - Point2f(0, 3), pa + Point2f(0, 2), ARGB(255, 0, 0, 255));
        }
        if (tri.vb >= numOriginalVertices)
        {
            iRenderQueue::getSingleton().render(pb - Point2f(3, 0), pb + Point2f(2, 0), ARGB(255, 0, 0, 255));
            iRenderQueue::getSingleton().render(pb - Point2f(0, 3), pb + Point2f(0, 2), ARGB(255, 0, 0, 255));
        }
        if (tri.vc >= numOriginalVertices)
        {
            iRenderQueue::getSingleton().render(pc - Point2f(3, 0), pc + Point2f(2, 0), ARGB(255, 0, 0, 255));
            iRenderQueue::getSingleton().render(pc - Point2f(0, 3), pc + Point2f(0, 2), ARGB(255, 0, 0, 255));
        }
#endif
#if ROUTEMAP_RENDERDEBUG_TRIANGLE_EDGES
#if ROUTEMAP_RENDERDEBUG_BLOCK_TRIANGLE_EDGES
        if (!tri.block)
#endif
        {
            // 宁可画2遍,也不愿意做一次检测...
            DWORD c1 = ARGB(alpha, 128, 255, 128);
            DWORD c2 = ARGB(alpha, 255, 255, 0);
            DWORD c3 = ARGB(alpha, 255, 0, 255);
            size_t eid = tri.e[Triangle::EdgeAB];
            DWORD c = c1;
            if (eid < edges.size())
            {
                if (edges[eid].bBad)
                    c = c2;
                else if (edges[eid].failedToTurn)
                    c = c3;
            }
            iRenderQueue::getSingleton().render(pa, pb, c);
            eid = tri.e[Triangle::EdgeBC];
            c = c1;
            if (eid < edges.size())
            {
                if (edges[eid].bBad)
                    c = c2;
                else if (edges[eid].failedToTurn)
                    c = c3;
            }
            iRenderQueue::getSingleton().render(pc, pb, c);
            eid = tri.e[Triangle::EdgeAC];
            c = c1;
            if (eid < edges.size())
            {
                if (edges[eid].bBad)
                    c = c2;
                else if (edges[eid].failedToTurn)
                    c = c3;
            }
            iRenderQueue::getSingleton().render(pa, pc, c);
        }
#endif

#if 0
        const Point2f p2[3] = {pa + da * -3, pb + db * -3, pc + dc * -3, };
        for (int i = 0; i < 3; i++)
        {
            //if (tri.t[i] == size_t(-1))
            {
                iRenderQueue::getSingleton().render(p2[(i + 2) % 3], p2[(i + 1) % 3], ARGB(alpha, 255, 128, 255));
            }
            /*
            else
            {
                assert(tri.t[i] < triangles.size());
                const Triangle &tOther = triangles[tri.t[i]];
                const Point2f &cOther = getCenter(tOther);
                iRenderQueue::getSingleton().render(center, cOther , ARGB(64, 0, 0, 128));
            }
            */
        }
#endif

#if ROUTEMAP_RENDERDEBUG_FILL_BLOCK_AREA
        // 障碍物，填充红色
        hgeTriple t;
        if (tri.block)
        {
            t.v[0].col = ARGB(64, 255, 0, 0);
            t.v[1].col = ARGB(64, 255, 0, 0);
            t.v[2].col = ARGB(64, 255, 0, 0);
        }
        else
        {
            t.v[0].col = ARGB(16, 0, 255, 0);
            t.v[1].col = ARGB(16, 0, 255, 0);
            t.v[2].col = ARGB(16, 0, 255, 0);
        }
        t.v[0].x = pa.x;
        t.v[0].y = pa.y;
        t.v[0].z = 0;
        t.v[0].tx = t.v[0].ty = 0;
        t.v[1].x = pb.x;
        t.v[1].y = pb.y;
        t.v[1].z = 0;
        t.v[1].tx = t.v[1].ty = 0;
        t.v[2].x = pc.x;
        t.v[2].y = pc.y;
        t.v[2].z = 0;
        t.v[2].tx = t.v[2].ty = 0;
        t.tex = 0;
        t.blend = BLEND_DEFAULT;
        iRenderQueue::getSingleton().render(Point2f(), t);
#endif
    }
    objects.clear();
    culler.dataType = DT_Edge;
    polySpace.cull(culler);

    HGE *hge = hgeCreate(HGE_VERSION);
    Point2f mousePos;
    hge->Input_GetMousePos(&mousePos.x, &mousePos.y);
    hge->Release();
    iRenderQueue::getSingleton().screenToGame(mousePos);
    vector<size_t> picked;
    MapObjCuller pickCuller(picked);
    pickCuller.viewer.leftTop = mousePos + Point2f(-10, -10);
    pickCuller.viewer.rightBottom = mousePos + Point2f(10, 10);
    pickCuller.dataType = DT_Edge;
    space.cull(pickCuller);

    iRenderQueue::getSingleton().render(pickCuller.viewer.leftTop, pickCuller.viewer.rightBottom, ARGB(255, 255, 0, 255));

    for (size_t p = 0; p < picked.size(); p++)
    {
        size_t peid = picked[p];
        assert(peid < edges.size());
        const Edge &pe = edges[peid];
        const Point2f &pea = verticesBuffer[pe.va];
        const Point2f &peb = verticesBuffer[pe.vb];
        iRenderQueue::getSingleton().render(pea, peb, ARGB(255, 255, 128, 128));
        Point2f crossPoint0;
        if (::LineCrossPoint(pea, peb, pickCuller.viewer.leftTop, pickCuller.viewer.rightBottom, crossPoint0))
        {
            for (size_t i = 0; i < objects.size(); i++)
            {
                size_t eid = objects[i];
                assert(eid < polyEdges.size());
                const Edge &e = polyEdges[eid];

                const Point2f &pa = verticesBuffer[e.va];
                const Point2f &pb = verticesBuffer[e.vb];

                Point2f crossPoint;
                if (LineCrossPoint(e, pe, crossPoint)) // 求交点
                {
                    iRenderQueue::getSingleton().render(pa, pb, ARGB(255, 255, 0, 255));
                    iRenderQueue::getSingleton().render(crossPoint + Point2f(-3, 0), crossPoint + Point2f(3, 0), ARGB(255, 255, 225, 255));
                    iRenderQueue::getSingleton().render(crossPoint + Point2f(0, -3), crossPoint + Point2f(0, 3), ARGB(255, 255, 225, 255));
                }
                else
                {
                    iRenderQueue::getSingleton().render(pa, pb, ARGB(255, 0, 255, 255));
                }
            }
        }
    }
}

#if 0
/* @fn generateMapFromVertices_old
   @note no longer used, just for reference
*/
void RouteMap::generateMapFromVertices_old()
{
    vector<Edge> newlines;
    size_t n = verticesBuffer.size();
    newlines.reserve(n * (n - 1) / 2);
    for (size_t i = 0; i < verticesBuffer.size(); i++)
    {
        for (size_t j = 0; j < i; j++)
        {
            Edge line;
            line.va = i;
            line.vb = j;
            line.len = (verticesBuffer[i] - verticesBuffer[j]).Length();
            newlines.push_back(line);
        }
    }
    sort(newlines.begin(), newlines.end());
    RouteMapObjSpace s(5);
    s.setRange(range);
    size_t numFace = (n - 3) * 2;
    size_t numEdge = n + numFace - 1;
    for (size_t i = 0; i < newlines.size(); i++)
    {
        const Edge &newline = newlines[i];
        RouteMapObjSpace::Node n;
        n.dataType = DT_Edge;
        n.dataId = edges.size();
        edges.push_back(newline); // 暂且先添加进去
        n.boundingBox.leftTop.x = verticesBuffer[newline.va].x;
        n.boundingBox.leftTop.y = verticesBuffer[newline.va].y;
        n.boundingBox.rightBottom.x = verticesBuffer[newline.vb].x;
        n.boundingBox.rightBottom.y = verticesBuffer[newline.vb].y;
        if (n.boundingBox.leftTop.x > n.boundingBox.rightBottom.x)
            swap(n.boundingBox.leftTop.x, n.boundingBox.rightBottom.x);
        if (n.boundingBox.leftTop.y > n.boundingBox.rightBottom.y)
            swap(n.boundingBox.leftTop.y, n.boundingBox.rightBottom.y);
        if (!s.checkCollision(n, *this))
            //if (j == lines.size())
        {
            // 前面暂且添加的,加对了
            s.addEntity(n.dataId, n.dataType, n.boundingBox);
            if (edges.size() >= numEdge)
                break;
        }
        else
        {
            // 前面暂且添加的不对.
            edges.pop_back();
        }
    }
}
#endif

bool RouteMapObjSpace::checkCollision(const Node &n, RouteMap &rmap)
{
    size_t size = entities.size();
    for (size_t i = 0; i < size; ++i)
    {
        if (entities[i].dataType != DT_Edge)
            continue;

        size_t line2 = entities[i].dataId;
        if (rmap.checkCollision(n.dataId, line2))
        {
            return true;
        }
    }

    if (child)
    {
        const Rectf &r = n.boundingBox;

        size_t cid = getChildId(r);
        if (cid < NumSubSpaces)
        {
            return getChild(cid).checkCollision(n, rmap);
        }
        else
        {
            assert(cid == size_t(-1));
            for (cid = 0; cid < NumSubSpaces; cid++)
            {
                if (getChild(cid).isIntersected(r) && getChild(cid).checkCollision(n, rmap))
                    return true;
            }
        }
    }

    return false;
}

void RouteMapObjSpace::cull(MapObjCuller &culler) const
{
    size_t size = entities.size();
    for (size_t i = 0; i < size; ++i)
    {
        const Node &node = entities[i];
        if (culler.filter(node.dataType) && (node.boundingBox & culler.viewer).Visible())
        {
            culler.objects.push_back(node.dataId);
        }
    }
    if (child)
    {
        size_t cid = getChildId(culler.viewer);
        if (cid < NumSubSpaces)
        {
            getChild(cid).cull(culler);
        }
        else
        {
            assert(cid == size_t(-1));
            for (cid = 0; cid < NumSubSpaces; cid++)
            {
                if (getChild(cid).isIntersected(culler.viewer))
                    getChild(cid).cull(culler);
            }
        }
    }
}

void RouteMap::BeginAddLine()
{
    verticesBuffer.clear();
    edges.clear();
    triangles.clear();
    range.leftTop = Point2f(0, 0);
    range.rightBottom = Point2f(0, 0);
}

void RouteMap::EndAddLine()
{
    RouteMapObjSpace s(5);
    s.setRange(range);

    for (size_t i = 0; i < edges.size(); i++)
    {
        const Edge &newline = edges[i];
        RouteMapObjSpace::Node n;
        n.dataType = DT_Edge;
        n.dataId = i;
        n.boundingBox.leftTop.x = verticesBuffer[newline.va].x;
        n.boundingBox.leftTop.y = verticesBuffer[newline.va].y;
        n.boundingBox.rightBottom.x = verticesBuffer[newline.vb].x;
        n.boundingBox.rightBottom.y = verticesBuffer[newline.vb].y;
        if (n.boundingBox.leftTop.x > n.boundingBox.rightBottom.x)
            swap(n.boundingBox.leftTop.x, n.boundingBox.rightBottom.x);
        if (n.boundingBox.leftTop.y > n.boundingBox.rightBottom.y)
            swap(n.boundingBox.leftTop.y, n.boundingBox.rightBottom.y);
        if (!s.checkCollision(n, *this))
        {
            s.addEntity(n.dataId, n.dataType, n.boundingBox);
        }
    }
}

void RouteMap::addLine(const Point2f &a, const Point2f &b)
{
    range.leftTop.x = min(range.leftTop.x, min(a.x, b.x));
    range.leftTop.y = min(range.leftTop.y, min(a.y, b.y));
    range.rightBottom.x = max(range.rightBottom.x, max(a.x, b.x));
    range.rightBottom.y = max(range.rightBottom.y, max(a.y, b.y));
    verticesBuffer.push_back(a);
    verticesBuffer.push_back(b);
    Edge line;
    line.va = verticesBuffer.size() - 2;
    line.vb = verticesBuffer.size() - 1;
    line.len = (a - b).Length();
    edges.push_back(line);
}
struct DistanceCmp
{
    struct Node
    {
        Node(size_t tid, size_t ptid, float _f) : triangleId(tid), parentTId(ptid), f(_f)
        {
        }
        size_t triangleId;
        size_t parentTId;

        float f;
    };
    DistanceCmp(const RouteMap &_rmap, const Point2f &_target) : rmap(_rmap), target(_target)
    {
    }
    const RouteMap &rmap;
    const Point2f &target;
    bool operator () (const Node &a, const Node &b) const
    {
        assert(a.triangleId < rmap.triangles.size());
        assert(b.triangleId < rmap.triangles.size());
        float da = a.f + (rmap.getCenter(rmap.triangles[a.triangleId]) - target).Length();
        float db = b.f + (rmap.getCenter(rmap.triangles[b.triangleId]) - target).Length();
        return da > db;
    }
};

bool RouteMap::aStarSearch(vector<size_t> &result, size_t from, size_t to) const
{
    assert(from < triangles.size());
    assert(to < triangles.size());
    assert(!triangles[from].block);
    assert(!triangles[to].block);

    const Point2f &target = getCenter(triangles[to]);

    typedef pair<size_t, float> VisitValue; // parent id, minimal distance
    typedef map<size_t, VisitValue> VisitMap;
    VisitMap visit;
    vector<DistanceCmp::Node> heap;
    heap.push_back(DistanceCmp::Node(from, size_t(-1), 0.f));
    visit.insert(pair<size_t, VisitValue>(from, VisitValue(size_t(-1), 0.f)));

    DistanceCmp distanceComparor(*this, target);

    while(!heap.empty())
    {
        pop_heap(heap.begin(), heap.end(), distanceComparor);
        DistanceCmp::Node n = heap.back();
        heap.pop_back();
        if (n.triangleId == to)
            break;

        assert(n.triangleId < triangles.size());
        const Triangle &tri0 = triangles[n.triangleId];
        const Point2f &ctri0 = getCenter(tri0);
        for (int i = 0; i < 3; i++)
        {
            if (tri0.t[i] == size_t(-1))
                continue;

            VisitMap::iterator iv = visit.find(tri0.t[i]);
            const Triangle &tri1 = triangles[tri0.t[i]];
            const Point2f &ctri1 = getCenter(tri1);
            float len = n.f + (ctri1 - ctri0).Length();
            if (iv == visit.end())
            {
                assert(tri0.t[i] != n.triangleId);
                visit.insert(iv, pair<size_t, VisitValue>(tri0.t[i], VisitValue(n.triangleId, len)));
                heap.push_back(DistanceCmp::Node(tri0.t[i], n.triangleId, len));
                push_heap(heap.begin(), heap.end(), distanceComparor);
            }
            else
            {
                if (len < iv->second.second)
                {
                    iv->second.first = n.triangleId;
                    iv->second.second = len;
                    assert(tri0.t[i] != n.triangleId);
                    heap.push_back(DistanceCmp::Node(tri0.t[i], n.triangleId, len));
                    push_heap(heap.begin(), heap.end(), distanceComparor);
                }
            }
        }
    }
    size_t id = to;
    result.clear();

    for(;;)
    {
        if (id == from)
            break;
        VisitMap::iterator ii = visit.find(id);
        if (ii == visit.end())
        {
            result.clear();
            break;
        }
        id = ii->second.first;
        result.push_back(id);
    }
    return id == from;
}

Point2f RouteMap::getCenter(const Triangle &t) const
{
    return (verticesBuffer[t.v[Triangle::PointA]] + verticesBuffer[t.v[Triangle::PointB]] + verticesBuffer[t.v[Triangle::PointC]]) / 3;
}

Rectf RouteMap::getBoundingBox(const Edge &e) const
{
    Rectf boundingBox(FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX);;
    {
        const Point2f &p = verticesBuffer[e.va];
        boundingBox.leftTop.x = min(p.x, boundingBox.leftTop.x);
        boundingBox.leftTop.y = min(p.y, boundingBox.leftTop.y);
        boundingBox.rightBottom.x = max(p.x, boundingBox.rightBottom.x);
        boundingBox.rightBottom.y = max(p.y, boundingBox.rightBottom.y);
    }
    {
        const Point2f &p = verticesBuffer[e.vb];
        boundingBox.leftTop.x = min(p.x, boundingBox.leftTop.x);
        boundingBox.leftTop.y = min(p.y, boundingBox.leftTop.y);
        boundingBox.rightBottom.x = max(p.x, boundingBox.rightBottom.x);
        boundingBox.rightBottom.y = max(p.y, boundingBox.rightBottom.y);
    }
    if (!boundingBox.Visible())
    {
        boundingBox.rightBottom += Point2f(0.1f, 0.1f);
    }
    return boundingBox;
}

Rectf RouteMap::getBoundingBox(const Triangle &t) const
{
    Rectf boundingBox(FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX);;
    for (int i = Triangle::PointA; i <= Triangle::PointC; i++)
    {
        const Point2f &p = verticesBuffer[t.v[i]];
        boundingBox.leftTop.x = min(p.x, boundingBox.leftTop.x);
        boundingBox.leftTop.y = min(p.y, boundingBox.leftTop.y);
        boundingBox.rightBottom.x = max(p.x, boundingBox.rightBottom.x);
        boundingBox.rightBottom.y = max(p.y, boundingBox.rightBottom.y);
    }
    if (!boundingBox.Visible())
    {
        boundingBox.rightBottom += Point2f(0.1f, 0.1f);
    }
    return boundingBox;
}

Rectf RouteMap::getBoundingBox(const Polygon &p) const
{
    Rectf boundingBox(FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX);
    for (size_t i = 0; i < p.vertices.size(); ++i)
    {
        assert(p.vertices[i] < verticesBuffer.size());
        const Point2f &pt = verticesBuffer[p.vertices[i]];
        boundingBox.leftTop.x = min(pt.x, boundingBox.leftTop.x);
        boundingBox.leftTop.y = min(pt.y, boundingBox.leftTop.y);
        boundingBox.rightBottom.x = max(pt.x, boundingBox.rightBottom.x);
        boundingBox.rightBottom.y = max(pt.y, boundingBox.rightBottom.y);
    }
    if (!boundingBox.Visible())
    {
        boundingBox.rightBottom += Point2f(0.1f, 0.1f);
    }
    return boundingBox;
}

void RouteMap::FindPath(list<Point2f> &result, const Point2f &from, const Point2f &to) const
{
    RouteMapTriangleDistanceCompare cmp(*this);
    cmp.filterDataType = DT_RouteTriangle;
    size_t idFrom = space.getClosestEntity(from, cmp);
    cmp.resetDistance();
    size_t idTo = space.getClosestEntity(to, cmp);
    if (idFrom < triangles.size() && idTo < triangles.size())
    {
        vector<size_t> nodes;
        if (aStarSearch(nodes, idFrom, idTo))
        {
            result.push_back(from);//getCenter(triangles[idFrom]));
            size_t size = nodes.size();
            for (size_t i = 0; i < size; i++)
                result.push_back(getCenter(triangles[nodes[size - 1 - i]]));
            result.push_back(to);//getCenter(triangles[idTo]));
        }
    }
}

void RouteMap::setRange(const Rectf &range)
{
    this->range = range;
    space.clear();
    space.setRange(range);
}
void RouteMap::randomVertices(const Rectf &range)
{
    verticesBuffer.clear();
    {
        size_t n = 2000;
        verticesBuffer.reserve(n);
        float width = range.GetWidth();
        float height = range.GetHeight();
        for (size_t i = 0; i < n - 4; i++)
        {
            verticesBuffer.push_back(range.leftTop + Point2f(width * rand() / RAND_MAX, height * rand() / RAND_MAX));
        }

        verticesBuffer.push_back(range.leftTop + Point2f(width * 0, height * 0));
        verticesBuffer.push_back(range.leftTop + Point2f(width * 0, height * 1));
        verticesBuffer.push_back(range.leftTop + Point2f(width * 1, height * 0));
        verticesBuffer.push_back(range.leftTop + Point2f(width * 1, height * 1));
    }
    generateMapFromVertices();
}

void RouteMap::addPolygon(const vector<Point2f> &polygonVertices)
{
    Polygon polygon;
    size_t base = verticesBuffer.size();
    Rectf boundingBox(FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX);
    for (size_t i = 0; i < polygonVertices.size(); ++i)
    {
        const Point2f &p = polygonVertices[i];
        boundingBox.leftTop.x = min(p.x, boundingBox.leftTop.x);
        boundingBox.leftTop.y = min(p.y, boundingBox.leftTop.y);
        boundingBox.rightBottom.x = max(p.x, boundingBox.rightBottom.x);
        boundingBox.rightBottom.y = max(p.y, boundingBox.rightBottom.y);

        verticesBuffer.push_back(polygonVertices[i]);
        polygon.vertices.push_back(base + i);
    }
    if (!boundingBox.Visible())
    {
        boundingBox.rightBottom += Point2f(0.1f, 0.1f);
    }
    space.addEntity(polygons.size(), DT_Poly, boundingBox);
    polygons.push_back(polygon);
}

bool RouteMap::insidePolygon(const Point2f &p, const Polygon &polygon) const
{
    int size = (int)polygon.vertices.size();
    int i0 = 0, i1 = 0;
    for (;;)
    {
        Point2f N, D;

        const Point2f &p1 = verticesBuffer[polygon.vertices[i1]];
        const Point2f &p0 = verticesBuffer[polygon.vertices[i0]];

        int iDiff = i1 - i0;
        if (iDiff == 1 || (iDiff < 0 && iDiff + size == 1))
        {
            N.x = p1.y - p0.y;
            N.y = p0.x - p1.x;
            D = p - p0;
            return N * D <= 0.0f;
        }

        // bisect the index range
        int iMid;
        if (i0 < i1)
        {
            iMid = (i0 + i1) >> 1;
        }
        else
        {
            iMid = ((i0 + i1 + size) >> 1);
            if (iMid >= size)
            {
                iMid -= size;
            }
        }
        const Point2f &pMid = verticesBuffer[polygon.vertices[iMid]];

        // determine which side of the splitting line contains the point
        N.x = pMid.y - p0.y;
        N.y = p0.x - pMid.x;
        D = p - p0;
        if (N * D > 0.0f)
        {
            // P potentially in <V(i0),V(i0+1),...,V(mid-1),V(mid)>
            i1 = iMid;
        }
        else
        {
            // P potentially in <V(mid),V(mid+1),...,V(i1-1),V(i1)>
            i0 = iMid;
        }
    }
}

void RouteMap::generateMapFromVertices()
{
    edges.clear();
    size_t i;

    numOriginalVertices = verticesBuffer.size();
    //vector<Edge> polyEdges;
    RouteMapObjSpace &tmpSpace = polySpace;
    tmpSpace.clear();
    tmpSpace.setRange(this->range);

    // generate extra vertices from polygons' intersected points
    for (i = 0; i < polygons.size(); i++)
    {
        const Polygon &polygon = polygons[i];
        vector<size_t> result;
        MapObjCuller culler(result);
        culler.dataType = DT_Edge;

        size_t sv;
        Edge e;
        // 对每条边，利用tmpSpace的cull找到与它有可能发生交叉的所有polyEdges的索引号
        e.va = polygon.vertices.back();
        for (sv = 0; sv < polygon.vertices.size(); sv++)
        {
            e.vb = polygon.vertices[sv];
            culler.viewer = getBoundingBox(e);
            tmpSpace.cull(culler);
            // 对这些polyEdges，判断是否与当前polygon的当前边有交点
            for (size_t j = 0; j < result.size(); j++)
            {
                Point2f crossPoint;
                if (LineCrossPoint(e, polyEdges[result[j]], crossPoint)) // 求交点
                {
                    // 若有交点，则该点也需要加入到三角剖分的计算顶点中去
                    verticesBuffer.push_back(crossPoint);
                }
            }
            e.va = e.vb;
        }

        // add polygon's edges to the tmpSpace
        e.va = polygon.vertices.back();
        for (sv = 0; sv < polygon.vertices.size(); sv++)
        {
            e.vb = polygon.vertices[sv];
            tmpSpace.addEntity(polyEdges.size(), DT_Edge, getBoundingBox(e));
            polyEdges.push_back(e);
            e.va = e.vb;
        }
    }

    // convert verticesBuffer to vertexSet
    vertexSet vs;
    for (i = 0; i < verticesBuffer.size(); i++)
    {
        vertex v(verticesBuffer[i].x, verticesBuffer[i].y);
        v.setUserData(i);
        vs.insert(v);
    }

    // call Delaunay methods
    triangleSet ts;
    Delaunay::Triangulate(vs, ts);
    edgeSet es;
    Delaunay::TrianglesToEdges(ts, es);

    int cc = 0;
    // fetch the result of Delaunay
    for (ctIterator cti = ts.begin(); cti != ts.end(); ++cti)
    {
        Triangle t;
        t.v[Triangle::PointA] = cti->GetVertex(0)->getUserData();
        t.v[Triangle::PointB] = cti->GetVertex(1)->getUserData();
        t.v[Triangle::PointC] = cti->GetVertex(2)->getUserData();
        sort(t.v, t.v + 3);

        // add triangle to space
        const Rectf &boundingBox = getBoundingBox(t);
        tmpSpace.addEntity(triangles.size(), DT_Triangle, boundingBox);
        triangles.push_back(t);
    }

    // generate edges, and set triangles' neighbourhood 
    for (cedgeIterator cei = es.begin(); cei != es.end(); ++cei)
    {
        Edge e;
        size_t eid = edges.size();
        e.va = cei->m_pV0->getUserData();
        e.vb = cei->m_pV1->getUserData();
        if (e.va > e.vb)
            swap(e.va, e.vb);
        e.t0 = cei->t0;
        e.t1 = cei->t1;
        if (e.t0 > e.t1)
            swap(e.t0, e.t1);
        assert(e.va < verticesBuffer.size());
        assert(e.vb < verticesBuffer.size());

        // connect 2 triangles
        size_t t0 = e.t0;
        size_t t1 = e.t1;
        if (t0 < triangles.size() && t1 < triangles.size())
        {
            Triangle &tri0 = triangles[t0];
            Triangle &tri1 = triangles[t1];
            //if (tri0.block || tri1.block)
            //    continue;

            // FIXME: what ugly code it is ...
#define SET_NEIGHBOUR_TRIANGLE(tri, t1)                                             \
    if (tri.v[Triangle::PointA] == e.va)                                            \
    {                                                                               \
        assert(tri.v[Triangle::PointB] == e.vb || tri.v[Triangle::PointC] == e.vb); \
        if (tri.v[Triangle::PointB] == e.vb)                                        \
            tri.t[Triangle::EdgeAB] = t1, tri.e[Triangle::EdgeAB] = eid;            \
        else                                                                        \
            tri.t[Triangle::EdgeAC] = t1, tri.e[Triangle::EdgeAC] = eid;            \
    }                                                                               \
    else if (tri.v[Triangle::PointB] == e.va)                                       \
    {                                                                               \
        assert(tri.v[Triangle::PointA] == e.vb || tri.v[Triangle::PointC] == e.vb); \
        if (tri.v[Triangle::PointA] == e.vb)                                        \
            tri.t[Triangle::EdgeAB] = t1, tri.e[Triangle::EdgeAB] = eid;            \
        else                                                                        \
            tri.t[Triangle::EdgeBC] = t1, tri.e[Triangle::EdgeBC] = eid;            \
    }                                                                               \
    else                                                                            \
    {                                                                               \
        assert(tri.v[Triangle::PointC] == e.va);                                    \
        assert(tri.v[Triangle::PointA] == e.vb || tri.v[Triangle::PointB] == e.vb); \
        if (tri.v[Triangle::PointA] == e.vb)                                        \
            tri.t[Triangle::EdgeAC] = t1, tri.e[Triangle::EdgeAC] = eid;            \
        else                                                                        \
            tri.t[Triangle::EdgeBC] = t1, tri.e[Triangle::EdgeBC] = eid;            \
    }

            SET_NEIGHBOUR_TRIANGLE(tri0, t1);
            SET_NEIGHBOUR_TRIANGLE(tri1, t0);
        }
//      space.addEntity(edges.size(), DT_Edge, getBoundingBox(e));
        edges.push_back(e);
    }

    // 对每条边检查是否与多边形的边有交叉，若有则需要turn
    vector<size_t> edgeIds(edges.size());
    for (i = 0; i < edges.size(); i++)
        edgeIds[i] = i;
    
    vector<size_t> edgeIds2;
    edgeIds2.reserve(edgeIds.size());
    size_t tryCount = 3; // 最多进行尝试3次无效的剔除
    while(!edgeIds.empty())
    {
        // 过滤edgeIds中的边，把与多边形边相交的边添加到edgeIds2中去
        for (i = 0; i < edgeIds.size(); i++)
        {
            size_t eid = edgeIds[i];
            Edge &e = edges[eid]; // 

            e.failedToTurn = false;

            vector<size_t> result; // id in polyEdges
            MapObjCuller culler(result);
            culler.dataType = DT_Edge;
            culler.viewer = getBoundingBox(e);
            tmpSpace.cull(culler);
            for (size_t j = 0; j < result.size(); j++)
            {
                const Edge &polyEdge = polyEdges[result[j]];
                if (e.va == polyEdge.va || e.va == polyEdge.vb || 
                    e.vb == polyEdge.va || e.vb == polyEdge.vb)
                    continue;

                // 如果一端恰好是该多边形边的切割点，那么也要忽略相交检测
                if (e.va >= numOriginalVertices)
                {
                    if (DistancePointToEdge(e.va, polyEdge) < 0.1f)
                        continue;
                }
                if (e.vb >= numOriginalVertices)
                {
                    if (DistancePointToEdge(e.vb, polyEdge) < 0.1f)
                        continue;
                }

                Point2f crossPoint;
                if (LineCrossPoint(e, polyEdge, crossPoint)) // 求交点
                {
                    edgeIds2.push_back(eid);
                    break;
                }
            }
        }

        if (!edgeIds2.empty() && edgeIds2.size() == edgeIds.size() && tryCount-- == 0)
        {
            for (i = 0; i < edgeIds2.size(); i++)
                edges[edgeIds2[i]].bBad = true;

            break;
        }

        // 尝试旋转这些边，如果旋转成功，则再检测一次
        for (i = 0; i < edgeIds2.size(); i++)
        {
            size_t eid = edgeIds2[i];
            // 如果无法旋转（比如边缘的边），那么不必再考虑它了
            // 不过实际情况是，边缘的边 *不可能* 与任何多边形相交。若相交，则它两侧必然存在顶点，于是会得到两侧的三角形。
            if (turnEdge(edges[eid]))
            {
                // 如果旋转成功，那么加入数组再检测一次
                //edgeIds.push_back(eid);
            }
            else
            {
                //edges[eid].failedToTurn = true;
            }
        }
        edgeIds = edgeIds2;
        edgeIds2.clear();
    }

    for (i = 0; i < edges.size(); i++)
    {
        const Edge &e = edges[i];
        space.addEntity(i, DT_Edge, getBoundingBox(e));
        if (e.t0 != size_t(-1))
        {
            assert(e.t0 < triangles.size());
            Triangle &t = triangles[e.t0];
            for (int j = 0; j < 3; j++)
            {
                if (t.e[j] == i)
                {
                    t.t[j] = e.t1;
                    break;
                }
            }
        }
        if (e.t1 != size_t(-1))
        {
            assert(e.t1 < triangles.size());
            Triangle &t = triangles[e.t1];
            for (int j = 0; j < 3; j++)
            {
                if (t.e[j] == i)
                {
                    t.t[j] = e.t0;
                    break;
                }
            }
        }
    }

    for (i = 0; i < triangles.size(); i++)
    {
        Triangle &t = triangles[i];
        const Point2f pos = getCenter(t);
        vector<size_t> result;
        MapObjCuller culler(result);
        culler.dataType = DT_Poly;
        culler.viewer.leftTop = pos + Point2f(-1.f, -1.f);
        culler.viewer.rightBottom = pos + Point2f(1.f, 1.f);
        space.cull(culler);
        t.block = 0;
        for (size_t j = 0; j < result.size(); j++)
        {
            size_t id = result[j];
            assert(id < polygons.size());
            if (insidePolygon(pos, polygons[id]) || 
                insidePolygon(lerp(verticesBuffer[t.v[Triangle::PointA]], pos, 0.1f), polygons[id]) || 
                insidePolygon(lerp(verticesBuffer[t.v[Triangle::PointB]], pos, 0.1f), polygons[id]) || 
                insidePolygon(lerp(verticesBuffer[t.v[Triangle::PointC]], pos, 0.1f), polygons[id]))
            {
                t.block = 1;
                cc++;
                break;
            }
        }
        space.addEntity(i, t.block ? DT_BlockTriangle : DT_RouteTriangle, getBoundingBox(t));
        t.t[0] = t.t[1] = t.t[2] = size_t(-1);
    }
    for (i = 0; i < edges.size(); i++)
    {
        const Edge &e = edges[i];
        // connect 2 triangles
        size_t t0 = e.t0;
        size_t t1 = e.t1;
        size_t eid = i;
        if (t0 < triangles.size() && t1 < triangles.size())
        {
            Triangle &tri0 = triangles[t0];
            Triangle &tri1 = triangles[t1];
            if (tri0.block || tri1.block)
                continue;

            SET_NEIGHBOUR_TRIANGLE(tri0, t1);
            SET_NEIGHBOUR_TRIANGLE(tri1, t0);
        }
    }
#undef SET_NEIGHBOUR_TRIANGLE
}

bool RouteMap::turnEdge(Edge &e)
{
    if (e.t0 == size_t(-1) || e.t1 == size_t(-1))
        return false;

    assert(e.t0 != e.t1);
    assert(e.t0 < triangles.size());
    assert(e.t1 < triangles.size());
    Triangle &tri0 = triangles[e.t0];
    Triangle &tri1 = triangles[e.t1];

    // turn the edge
    size_t va = e.va, vb = e.vb;
    const size_t *v0 = tri0.v, *v1 = tri1.v; // 两个三角形各自独立的点
#define GET_SINGLE_VERTEX(tri, vx)                                                  \
    if (tri.v[Triangle::PointA] == va)                                              \
    {                                                                               \
        assert(tri.v[Triangle::PointB] == vb || tri.v[Triangle::PointC] == vb);     \
        vx += (tri.v[Triangle::PointB] == vb ? Triangle::PointC : Triangle::PointB);\
    }                                                                               \
    else if (tri.v[Triangle::PointB] == va)                                         \
    {                                                                               \
        assert(tri.v[Triangle::PointA] == vb || tri.v[Triangle::PointC] == vb);     \
        vx += (tri.v[Triangle::PointA] == vb ? Triangle::PointC : Triangle::PointA);\
    }                                                                               \
    else                                                                            \
    {                                                                               \
        assert(tri.v[Triangle::PointC] == va);                                      \
        assert(tri.v[Triangle::PointA] == vb || tri.v[Triangle::PointB] == vb);     \
        vx += (tri.v[Triangle::PointA] == vb ? Triangle::PointB : Triangle::PointA);\
    }

    GET_SINGLE_VERTEX(tri0, v0);
    GET_SINGLE_VERTEX(tri1, v1);
#undef GET_SINGLE_VERTEX

    {
        const Point2f &pa = verticesBuffer[va];
        const Point2f &pb = verticesBuffer[vb];
        const Point2f &p0 = verticesBuffer[*v0];
        const Point2f &p1 = verticesBuffer[*v1];
        const Point2f a1 = pa - p1;
        const Point2f b1 = pb - p1;
        const Point2f _01 = p0 - p1;
        if ((a1 ^ _01) * (_01 ^ b1) < 0) // 不能转到三角形之外。
            return false;
    }

    e.va = *v0;
    e.vb = *v1;
    if (e.va > e.vb)
        swap(e.va, e.vb);

    int i0c = 0, i1c = 0; // 两个三角形需要交换的邻接三角形和
    // turn the triangles
#define SWAP_AND_GET_EDGE(_tri, _vb, _va, _i0c, _v1)                                    \
    if (_tri.v[Triangle::PointA] == _vb)                                                \
    {                                                                                   \
        _tri.v[Triangle::PointA] = *_v1;                                                \
        /*swap(_tri.t[Triangle::EdgeAC], _tri.t[Triangle::EdgeAB]); */                  \
        swap(_tri.e[Triangle::EdgeAC], _tri.e[Triangle::EdgeAB]);                       \
        assert(_tri.v[Triangle::PointB] == _va || _tri.v[Triangle::PointC] == _va);     \
        _i0c = _tri.v[Triangle::PointB] == _va ? Triangle::EdgeAB : Triangle::EdgeAC;   \
    }                                                                                   \
    else if (_tri.v[Triangle::PointB] == _vb)                                           \
    {                                                                                   \
        _tri.v[Triangle::PointB] = *_v1;                                                \
        /*swap(_tri.t[Triangle::EdgeBC], _tri.t[Triangle::EdgeAB]); */                  \
        swap(_tri.e[Triangle::EdgeBC], _tri.e[Triangle::EdgeAB]);                       \
        assert(_tri.v[Triangle::PointA] == _va || _tri.v[Triangle::PointC] == _va);     \
        _i0c = _tri.v[Triangle::PointA] == _va ? Triangle::EdgeAB : Triangle::EdgeBC;   \
    }                                                                                   \
    else                                                                                \
    {                                                                                   \
        assert(_tri.v[Triangle::PointC] == _vb);                                        \
        _tri.v[Triangle::PointC] = *_v1;                                                \
        /*swap(_tri.t[Triangle::EdgeAC], _tri.t[Triangle::EdgeBC]); */                  \
        swap(_tri.e[Triangle::EdgeAC], _tri.e[Triangle::EdgeBC]);                       \
        assert(_tri.v[Triangle::PointB] == _va || _tri.v[Triangle::PointA] == _va);     \
        _i0c = _tri.v[Triangle::PointB] == _va ? Triangle::EdgeBC : Triangle::EdgeAC;   \
    }

    SWAP_AND_GET_EDGE(tri0, vb, va, i0c, v1);
    SWAP_AND_GET_EDGE(tri1, va, vb, i1c, v0);
#undef SWAP_AND_GET_EDGE

    //swap(tri0.t[i0c], tri1.t[i1c]);
    swap(tri0.e[i0c], tri1.e[i1c]);

#define SET_EDGE_NEIGHBOUT_TRIANGLE(tri, i)                                         \
    if (tri.e[i] != size_t(-1))                                                     \
    {                                                                               \
        Edge &e1 = edges[tri.e[i]];                                                 \
        assert(e1.t0 == e.t1 || e1.t1 == e.t1 || e1.t0 == e.t0 || e1.t1 == e.t0);   \
        if (e1.t0 == e.t1)                                                          \
        {                                                                           \
            assert(e1.t1 != e.t0);                                                  \
            e1.t0 = e.t0;                                                           \
        }                                                                           \
        else if (e1.t1 == e.t1)                                                     \
        {                                                                           \
            assert(e1.t0 != e.t0);                                                  \
            e1.t1 = e.t0;                                                           \
        }                                                                           \
        else if (e1.t0 == e.t0)                                                     \
        {                                                                           \
            assert(e1.t1 != e.t0);                                                  \
            e1.t0 = e.t1;                                                           \
        }                                                                           \
        else if (e1.t1 == e.t0)                                                     \
        {                                                                           \
            assert(e1.t0 != e.t0);                                                  \
            e1.t1 = e.t1;                                                           \
        }                                                                           \
        assert(e1.t0 != e1.t1);                                                     \
    }
    SET_EDGE_NEIGHBOUT_TRIANGLE(tri0, i0c);
    SET_EDGE_NEIGHBOUT_TRIANGLE(tri1, i1c);
#undef SET_EDGE_NEIGHBOUT_TRIANGLE

    return true;
}
