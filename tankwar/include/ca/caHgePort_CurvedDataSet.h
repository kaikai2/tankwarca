#ifndef _caHgePort_CurvedDataSet_H_
#define _caHgePort_CurvedDataSet_H_

#include "caHgePort_define.h"

// Êý¾Ý¼¯
template<class T>
class hgeCurveDataSet : public iCurveDataSet
{
public:
    virtual const char *getTypeName() const;
    virtual bool loadFromByteStream(iByteStream *bs)
    {
        return iCurveDataSet::loadFromByteStream(bs);
    }
    virtual bool saveToByteStream(iByteStream *bs) const
    {
        return iCurveDataSet::saveToByteStream(bs);
    }
    virtual size_t getDataCount() const
    {
        return data.size();
    }
    virtual void resize(size_t index)
    {
        data.resize(index);
    }
    virtual void setData(size_t index, const void *value, int time)
    {
        assert(index < data.size());
        data[index].data = *(T*)value;
        data[index].time = time;
    }
    virtual void setData(size_t index, const void *value)
    {
        assert(index < data.size());
        data[index].data = *(T*)value;
    }
    virtual void setTime(size_t index, int time)
    {
        assert(index < data.size());
        data[index].time = time;
    }
    virtual int getTime(size_t index) const
    {
        assert(index < data.size());
        return data[index].time;
    }
    virtual const void *getData(size_t index) const
    {
        assert(index < data.size());
        return &data[index].data;
    }
    virtual size_t getDataSize() const
    {
        return sizeof(T);
    }
    virtual void setDataString(size_t index, const char *valueStr);
    virtual const char *getDataString(size_t index) const;
    struct DataNode
    {
        int time;
        T data;
    };
	std::vector<DataNode> data;
};

#endif//_caHgePort_CurvedDataSet_H_
