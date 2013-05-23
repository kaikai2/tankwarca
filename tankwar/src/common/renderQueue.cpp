#include <hgeFont.h>
#include <hgeCurvedani.h>
#include <caRect.h>
#include "common/renderQueue.h"
#include "common/graphicEntity.h"

void renderFontWithBk(hgeFont &font, int x, int y, DWORD bcolor, DWORD color, int align, const char *str)
{
    font.SetColor(bcolor);
    font.Render((float)(x-1),(float)(y-1),align,str);
    font.Render((float)(x-1),(float)(y),align,str);
    font.Render((float)(x-1),(float)(y+1),align,str);
    font.Render((float)(x),(float)(y-1),align,str);
    font.Render((float)(x),(float)(y+1),align,str);
    font.Render((float)(x+1),(float)(y-1),align,str);
    font.Render((float)(x+1),(float)(y),align,str);
    font.Render((float)(x+1),(float)(y+1),align,str);
    font.SetColor(color);
    font.Render((float)(x),(float)(y),align,str);
}

RenderQueue::RenderQueue() : viewer(0)
{

}
void RenderQueue::render(const Point2f &pos, float direction, int frame, const iAnimation *anim, size_t layer/* = LayerDebugInfo*/, bool screenPos/* = false*/)
{
    assert(layer < MaxLayers);
    if (!anim)
        return;

    GfxObj obj;
    obj.type = GfxObj::T_Anim;
    obj.data.anim.screenPos = screenPos;
    obj.data.anim.iA = gfxParams.addPoint2f(pos);
    obj.data.anim.direction = direction;
    obj.data.anim.frame = frame;
    obj.data.anim.anim = anim;
    gfxobjs[layer].push_back(obj);
}
void RenderQueue::render(const hgeQuad &quad, size_t layer/* = LayerDebugInfo*/, bool screenPos/* = false*/)
{
    assert(layer < MaxLayers);
    GfxObj obj;
    obj.type = GfxObj::T_Quad;
    obj.data.quad.screenPos = screenPos;
    obj.data.quad.iQuad = gfxParams.addQuad(quad);
    gfxobjs[layer].push_back(obj);
}
void RenderQueue::render(const Point2f &a, const Point2f &b, DWORD color, size_t layer/* = LayerDebugInfo*/, bool screenPos/* = false*/)
{
    assert(layer < MaxLayers);
    GfxObj obj;
    obj.type = GfxObj::T_Line;
    obj.data.line.screenPos = screenPos;
    obj.data.line.iA = gfxParams.addPoint2f(a);
    obj.data.line.iB = gfxParams.addPoint2f(b);
    obj.data.line.color = color;
    gfxobjs[layer].push_back(obj);
}
void RenderQueue::render(const Point2f &p, hgeFont *font, const char *s, int align, DWORD color, size_t layer/* = LayerDebugInfo*/, bool screenPos/* = false*/)
{
    assert(layer < MaxLayers);
    if (!font || !s)
        return;

    GfxObj obj;
    obj.type = GfxObj::T_Text;
    obj.data.text.screenPos = screenPos;
    obj.data.text.iA = gfxParams.addPoint2f(p);
    obj.data.text.font = font;
    obj.data.text.iStr = gfxParams.addString(s);
    obj.data.text.color = color;
    obj.data.text.align = align;
    gfxobjs[layer].push_back(obj);
}
void RenderQueue::render(const Point2f &p, const hgeTriple &t, size_t layer/* = LayerDebugInfo*/, bool screenPos/* = false*/)
{
    assert(layer < MaxLayers);
    GfxObj obj;
    obj.type = GfxObj::T_Triple;
    obj.data.triple.iA = gfxParams.addPoint2f(p);
    obj.data.triple.screenPos = screenPos;
    obj.data.triple.iTriple = gfxParams.addTriple(t);
    gfxobjs[layer].push_back(obj);
}
void RenderQueue::setViewer(Entity *entity)
{
    if (entity)
        viewer = (iViewerEntity *)entity->getEntityInterface(Entity::EII_ViewerEntity);
    else
        viewer = 0;
}
void RenderQueue::screenToGame(Point2f &pos)
{
    pos += getViewerPos();
}

void RenderQueue::setViewerPos(const Point2f &vp)
{
    viewerPos = vp;
}
const Point2f &RenderQueue::getWindowSize() const
{
    return windowSize;
}
void RenderQueue::setWindowSize(const Point2f &ws)
{
    windowSize = ws;
}

Point2f RenderQueue::getViewerPos()
{
    if (viewer)
    {
        viewerPos = viewer->getViewerPos() - windowSize / 2;
    }
    return viewerPos;
}

Rectf RenderQueue::getWindowViewer()
{
    Rectf r;
    r.leftTop = getViewerPos();
    r.rightBottom = windowSize + r.leftTop;
    return r; 
}

void RenderQueue::flush()
{
    HGE *hge = hgeCreate(HGE_VERSION);
    const Point2f viewerPos = getViewerPos();
    const Rectf viewRange = getWindowViewer();
    const Rectf windowRange(0, getWindowSize().x, 0, getWindowSize().y);


    //Rect clip;
    //clip.leftTop = Point2s(100, 100);
    //clip.rightBottom = Point2s(200, 200);
    hge->Gfx_SetTransform();
    for (int i = MaxLayers - 1; i >= 0; --i)
    {
        vector<GfxObj> &go = gfxobjs[i];
        for (size_t j = 0; j < go.size(); ++j)
        {
            const GfxObj &obj = go[j];
            switch(obj.type)
            {
            case GfxObj::T_Anim:
                {
                    const GfxObj::Data::Anim &anim = obj.data.anim;

                    Point2f pos = gfxParams.getPoint2f(anim.iA);
                    if (!anim.screenPos)
                        pos -= viewerPos;
                    // FIXME: 100 is magic number, remove it by anim clip range
                    if (pos.x > windowRange.rightBottom.x + 100 || pos.x < windowRange.leftTop.x - 100 ||
                        pos.y < windowRange.leftTop.y - 100 || pos.y > windowRange.rightBottom.y + 100)
                        break;
                    //hge->Gfx_SetTransform();
                    hge->Gfx_SetTransform(0, 0, pos.x, pos.y, anim.direction/* - viewerOrientation*/, 1, 1);
                    anim.anim->render(anim.frame, 0);//&clip);
                    hge->Gfx_SetTransform();
                }
                break;
            case GfxObj::T_Line:
                {
                    const GfxObj::Data::Line &line = obj.data.line;

                    const Point2f &posA = gfxParams.getPoint2f(line.iA);
                    const Point2f &posB = gfxParams.getPoint2f(line.iB);
                    Rectf objRange(
                        min(posA.x, posB.x + .1f),
                        max(posA.x, posB.x + .1f),
                        min(posA.y, posB.y + .1f),
                        max(posA.y, posB.y + .1f));
                    if (line.screenPos)
                    {
                        Rectf x = objRange & windowRange;
                        if (x.Visible())
                        {
                            hge->Gfx_RenderLine(posA.x, posA.y,
                                posB.x, posB.y, line.color);
                        }
                    }
                    else
                    {
                        Rectf x = objRange & viewRange;
                        if (x.Visible())
                        {
                            hge->Gfx_RenderLine(posA.x - viewerPos.x, posA.y - viewerPos.y,
                                posB.x - viewerPos.x, posB.y - viewerPos.y, line.color);
                        }
                    }
                }
                break;
            case GfxObj::T_Text:
                {
                    const GfxObj::Data::Text &text = obj.data.text;
                    
                    Point2f posA = gfxParams.getPoint2f(text.iA);
                    if (!text.screenPos)
                    {
                        posA -= viewerPos;
                    }
                    posA = SnapNearestInteger(posA);
                    //text.font->Render(posA.x, posA.y, text.align, gfxParams.getString(text.iStr).c_str());
                    renderFontWithBk(*text.font, posA.x, posA.y, ~text.color & 0x00ffffff | 0x40000000, text.color, text.align, gfxParams.getString(text.iStr).c_str());

                }
                break;
            case GfxObj::T_Triple:
                {
                    const GfxObj::Data::Triple &triple = obj.data.triple;
                    const Point2f &posA = gfxParams.getPoint2f(triple.iA);
                    if (triple.screenPos)
                        hge->Gfx_SetTransform(0, 0, posA.x, posA.y, 0, 1, 1);
                    else
                        hge->Gfx_SetTransform(0, 0, posA.x - viewerPos.x, posA.y - viewerPos.y, 0, 1, 1);

                    hge->Gfx_RenderTriple(&gfxParams.getTriple(triple.iTriple));
                    hge->Gfx_SetTransform();
                }
                break;
            case GfxObj::T_Quad:
                {
                    const GfxObj::Data::Quad &quad = obj.data.quad;
                    hge->Gfx_RenderQuad(&gfxParams.getQuad(quad.iQuad));
                }
                break;
            }
        }
        go.clear();
    }
    hge->Gfx_SetTransform();
    hge->Release();
    gfxParams.clear();
}
