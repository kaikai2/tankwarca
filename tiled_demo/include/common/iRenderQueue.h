#ifndef COMMON_I_RENDER_QUEUE_H
#define COMMON_I_RENDER_QUEUE_H

#include "hge/hge.h"
#include "hge/hgefont.h"
#include "utils.h"
#include "entity.h"
#include <caPoint2d.h>
#include <curvedani.h>
using namespace cAni;


class iRenderQueue : public Singleton<iRenderQueue>
{
public:
    enum
    {
        LayerDebugInfo = 0,
        MaxLayers = 16,
    };
    virtual void render(const Point2f &pos, float direction, int frame, const iAnimation *anim, size_t layer = LayerDebugInfo, bool screenPos = false) = 0;
    virtual void render(const hgeQuad &quad, size_t layer = LayerDebugInfo, bool screenPos = false) = 0;
    virtual void render(const Point2f &a, const Point2f &b, DWORD color, size_t layer = LayerDebugInfo, bool screenPos = false) = 0;
    virtual void render(const Point2f &p, hgeFont *font, const char *s, int align, DWORD color, size_t layer = LayerDebugInfo, bool screenPos = false) = 0;
    virtual void render(const Point2f &p, const hgeTriple &t, size_t layer = LayerDebugInfo, bool screenPos = false) = 0;
    virtual void flush() = 0;
    virtual void setViewer(Entity *entity) = 0;
    virtual Rectf getWindowViewer() = 0;
    virtual void screenToGame(Point2f &pos) = 0;
    virtual Point2f getViewerPos() = 0;
    virtual void setViewerPos(const Point2f &vp) = 0;
    virtual const Point2f &getWindowSize() const = 0;
    virtual void setWindowSize(const Point2f &ws) = 0;
};

#endif//COMMON_I_RENDER_QUEUE_H
