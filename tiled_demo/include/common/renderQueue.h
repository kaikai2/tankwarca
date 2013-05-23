#ifndef COMMON_RENDER_QUEUE_H
#define COMMON_RENDER_QUEUE_H

#include "utils.h"
#include "entity.h"
#include "iRenderQueue.h"
#include <caPoint2d.h>
#include <curvedani.h>
#include <vector>
#include <string>
using namespace cAni;

using cAni::Rectf;

class iViewerEntity;

struct GfxObj
{
public:
    enum Type
    {
        T_Anim,
        T_Line,
        T_Text,
        T_Triple,
        T_Quad,

        NumTypes,
    };
    Type type;
    union Data
    {
        struct Anim
        {
            bool screenPos;
            size_t iA;
            float direction;
            int frame;
            const iAnimation *anim;
        }anim;
        struct Quad
        {
            bool screenPos;
            size_t iQuad;
        }quad;
        struct Line
        {
            bool screenPos;
            size_t iA, iB;
            DWORD color;
        }line;
        struct Text
        {
            bool screenPos;
            size_t iA;
            size_t iStr;
            hgeFont *font;
            int align;
            DWORD color;
        }text;
        struct Triple
        {
            bool screenPos;
            size_t iA;
            size_t iTriple;
        } triple;
    }data;
    struct Param
    {
        std::vector<Point2f> vecPoint2f;
        std::vector<hgeTriple> vecTriple;
        std::vector<hgeQuad> vecQuad;
        std::vector<std::string> vecString;
        void clear()
        {
            vecPoint2f.clear();
            vecTriple.clear();
            vecQuad.clear();
            vecString.clear();
        }
        size_t addPoint2f(const Point2f &p)
        {
            size_t index = vecPoint2f.size();
            vecPoint2f.push_back(p);
            return index;
        }
        size_t addTriple(const hgeTriple &t)
        {
            size_t index = vecTriple.size();
            vecTriple.push_back(t);
            return index;
        }
        size_t addQuad(const hgeQuad &q)
        {
            size_t index = vecQuad.size();
            vecQuad.push_back(q);
            return index;
        }
        size_t addString(const std::string &q)
        {
            size_t index = vecString.size();
            vecString.push_back(q);
            return index;
        }
        const Point2f &getPoint2f(size_t index) const
        {
            return vecPoint2f[index];
        }
        const hgeTriple &getTriple(size_t index) const
        {
            return vecTriple[index];
        }
        const hgeQuad &getQuad(size_t index) const
        {
            return vecQuad[index];
        }
        const std::string &getString(size_t index) const
        {
            return vecString[index];
        }
    };
};



class RenderQueue : public iRenderQueue
{
public:
    RenderQueue();

    virtual void render(const Point2f &pos, float direction, int frame, const iAnimation *anim, size_t layer = LayerDebugInfo, bool screenPos = false);
    virtual void render(const hgeQuad &quad, size_t layer = LayerDebugInfo, bool screenPos = false);
    virtual void render(const Point2f &a, const Point2f &b, DWORD color, size_t layer = LayerDebugInfo, bool screenPos = false);
    virtual void render(const Point2f &p, hgeFont *font, const char *s, int align, DWORD color, size_t layer = LayerDebugInfo, bool screenPos = false);
    virtual void render(const Point2f &p, const hgeTriple &t, size_t layer = LayerDebugInfo, bool screenPos = false);
    virtual void flush();
    virtual void setViewer(Entity *entity);
    virtual Rectf getWindowViewer();
    virtual void screenToGame(Point2f &pos);
    virtual Point2f getViewerPos();
    virtual void setViewerPos(const Point2f &vp);
    virtual const Point2f &getWindowSize() const;
    virtual void setWindowSize(const Point2f &ws);
private:
    std::vector<GfxObj> gfxobjs[MaxLayers];
    iViewerEntity *viewer;
    Point2f viewerPos;
    Point2f windowSize;

    GfxObj::Param gfxParams;
};

#endif//COMMON_RENDER_QUEUE_H
