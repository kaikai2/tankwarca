#ifndef _CaHgePort_Texture_H_
#define _CaHgePort_Texture_H_

#include "caHgePort_define.h"

/// 纹理提供者
class hgeTexture : public iTexture
{
public:
    hgeTexture() : tex(0)
    {
        if (textureCount++ == 0)
        {
            hge = hgeCreate(HGE_VERSION);
        }
    }
    virtual ~hgeTexture()
    {
        if (tex)
        {
            hge->Texture_Free(tex);
        }
        if (--textureCount == 0)
        {
            hge->Release();
        }
    }
    virtual size_t getWidth() const
    {
        if (tex)
        {
            return size_t(hge->Texture_GetWidth(tex));
        }
        return 0;
    }
    virtual size_t getHeight() const
    {
        if (tex)
        {
            return size_t(hge->Texture_GetHeight(tex));
        }
        return 0;
    }
    HTEXTURE getHgeTexture()
    {
        if (!tex)
        {
            loadTexture(this->getFilename());
        }
        return tex;
    }
    void loadTexture(const char *filename)
    {
        if (tex)
        {
            hge->Texture_Free(tex);
        }
        tex = hge->Texture_Load(filename);
    }
protected:
    HTEXTURE tex;
    static HGE *hge;
    static size_t textureCount;
};

#endif//_CaHgePort_Texture_H_
