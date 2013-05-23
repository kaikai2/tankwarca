#ifndef _caHgePort_ByteStream_H_
#define _caHgePort_ByteStream_H_

#include "caHgePort_define.h"

class hgeByteStream : public iByteStream
{
public:
    hgeByteStream() : data(0), size(0)
    {
        if (byteStreamCount++ == 0)
        {
            hge = hgeCreate(HGE_VERSION);
        }
    }
    virtual ~hgeByteStream()
    {
        if (data)
        {
            hge->Resource_Free(data);
            size = 0;
        }
        if (--byteStreamCount == 0)
        {
            hge->Release();
        }
    }
    virtual bool write(const void *buf, size_t len)
    {
        buf, len;
        assert(0 && "not allowed to write");
        return false;
    }
    virtual bool read(void *buf, size_t len)
    {
        if (len > size - cpos)
            return false;
        if (len > 0)
        {
            memcpy(buf, data + cpos, len);
            cpos += (DWORD)len;
        }
        return true;
    }
    /// seek to position
    virtual void seek(size_t pos)
    {
        cpos = min(DWORD(pos), size);
    }
    /// seek with an offset
    virtual void seekOffset(int offset)
    {
        seek(cpos + offset);
    }
    virtual size_t length() const
    {
        return size;
    }
    virtual size_t tell() const
    {
        return cpos;
    }
    void loadByteStream(const char *name)
    {
        if (data)
        {
            hge->Resource_Free(data);
            size = 0;
        }
        data = (char *)hge->Resource_Load(name, &size);
        cpos = 0;
    }

protected:
    char *data;
    DWORD size;
    DWORD cpos;
    static HGE *hge;
    static size_t byteStreamCount;
};

#endif//_caHgePort_ByteStream_H_
