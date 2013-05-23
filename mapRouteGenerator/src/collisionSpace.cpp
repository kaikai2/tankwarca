#include <cassert>
#include <vector>
#include <algorithm>
#include "collisionSpace.h"

using std::vector;
using std::find;
using std::swap;

bool Map::LineCross(const Map::Line &l1, const Map::Line &l2, const vector<hgeVector> &vertices)
{
    const hgeVector &va = vertices[l1.va];
    const hgeVector &vb = vertices[l1.vb];
    const hgeVector &vc = vertices[l2.va];
    const hgeVector &vd = vertices[l2.vb];

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

    hgeVector dac = vc - va;
    hgeVector dab = vb - va;
    hgeVector dad = vd - va;
    hgeVector dcb = vb - vc;
    hgeVector dcd = vd - vc;
    hgeVector dca = va - vc;

    return CrossProduct(dac, dab) * CrossProduct(dab, dad) > 0 && 
        CrossProduct(dcb, dcd) * CrossProduct(dcd, dca) > 0;
}
const float w = 200;
const float h = 200;
void Map::render()
{
    float alpha = 255;
    float basex = (800 - w) / 2;
    float basey = (600 - h) / 2;
    for (size_t i = 0; i < lines.size(); i++)
    {
        const Line &line = lines[i];

        hge->Gfx_RenderLine(vertices[line.va].x + basex, vertices[line.va].y + basey,
            vertices[line.vb].x + basex, vertices[line.vb].y + basey,
            ARGB(alpha, 128, 255, 128));

        alpha -= 0.01;
        if (alpha <= 0)
            break;
    }
    for (size_t i = 0; i < vertices.size(); i++)
    {
        int x = vertices[i].x + 0.5;
        int y = vertices[i].y + 0.5;
        hge->Gfx_RenderLine(x - 3 + basex, y + basey, x + 2 + basex, y + basey, ARGB(255, 255, 255, 255));
        hge->Gfx_RenderLine(x + basex, y - 3 + basey, x + basex, y + 2 + basey, ARGB(255, 255, 255, 255));
    }
}
void Map::randomVertices()
{
    vertices.clear();
    lines.clear();

    size_t n = 500;
    vertices.reserve(n);

    for (int i = 0; i < n - 4; i++)
    {
        vertices.push_back(hgeVector(w * rand() / RAND_MAX, h * rand() / RAND_MAX));
    }

    vertices.push_back(hgeVector(w * 0, h * 0));
    vertices.push_back(hgeVector(w * 0, h * 1));
    vertices.push_back(hgeVector(w * 1, h * 0));
    vertices.push_back(hgeVector(w * 1, h * 1));
    vector<Line> newlines;
    newlines.reserve(n * (n - 1) / 2);
    for (size_t i = 0; i < vertices.size(); i++)
    {
        for (size_t j = i + 1; j < vertices.size(); j++)
        {
            Line line;
            line.va = i;
            line.vb = j;
            line.len = (vertices[i] - vertices[j]).Length();
            newlines.push_back(line);
        }
    }
    sort(newlines.begin(), newlines.end());
    QuadSpace s(7);
    s.setRange(Rectf(0, w, 0, h));
    size_t numFace = (n - 3) * 2;
    size_t numEdge = n + numFace - 1;
    for (size_t i = 0; i < newlines.size(); i++)
    {
        size_t j;
        const Line &newline = newlines[i];
        /*
        for (j = 0; j < lines.size(); j++)
        {
            if (LineCross(newline, lines[j], vertices))
                break;
        }
        */
        QuadSpace::Node n;
        n.line = newline;
        n.boundingBox.leftTop.x = vertices[newline.va].x;
        n.boundingBox.leftTop.y = vertices[newline.va].y;
        n.boundingBox.rightBottom.x = vertices[newline.vb].x;
        n.boundingBox.rightBottom.y = vertices[newline.vb].y;
        if (n.boundingBox.leftTop.x > n.boundingBox.rightBottom.x)
            swap(n.boundingBox.leftTop.x, n.boundingBox.rightBottom.x);
        if (n.boundingBox.leftTop.y > n.boundingBox.rightBottom.y)
            swap(n.boundingBox.leftTop.y, n.boundingBox.rightBottom.y);
        if (!s.checkCollision(n, *this))
        //if (j == lines.size())
        {
            lines.push_back(newline);
            s.addEntity(newline, n.boundingBox);
            if (lines.size() >= numEdge)
                break;
        }
    }
}
bool QuadSpace::checkCollision(const Node &n, CollisionChecker &collisionChecker)
{
    size_t size = entities.size();
    for (size_t i = 0; i < size; ++i)
    {
        Map::Line &line2 = entities[i].line;
        if (collisionChecker.checkCollision(n.line, line2))
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
            return child[cid].checkCollision(n, collisionChecker);
        }
        else
        {
            assert(cid == size_t(-1));
            for (cid = 0; cid < NumSubSpaces; cid++)
            {
                if (child[cid].isIntersected(r) && child[cid].checkCollision(n, collisionChecker))
                    return true;
            }
        }
    }

    return false;
}
