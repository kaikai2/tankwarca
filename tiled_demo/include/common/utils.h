#ifndef UTILS_H
#define UTILS_H

#include <cstdlib>
#include <cmath>
#include <cassert>

#include "caPoint2d.h"

using cAni::Point2f;

#pragma warning(disable: 4511 4512) // copy constructor could not be generated, assignment operator could not be generated

// not allowed to copy
class NoCopy
{
protected:
    NoCopy()
    {
    }
private:
    NoCopy(const NoCopy &o)
    {
        o;
    }
    NoCopy &operator = (const NoCopy &o)
    {
        return o, *this;
    }
};

// singleton
template<class T>
class Singleton : public virtual NoCopy
{
    static T *ms_Singleton;
public:
    Singleton()
    {
        assert(ms_Singleton == 0);
        ms_Singleton = static_cast<T *>(this);
    }
    virtual ~Singleton()
    {
        assert(ms_Singleton);
        ms_Singleton = 0;
    }
    static T& getSingleton()
    {
        assert(ms_Singleton);
        return *ms_Singleton;
    }
    static T* getSingletonPtr()
    {
        return ms_Singleton;
    }
    virtual void releaseSingleton()
    {
    }
};

template<class T>
T *Singleton<T>::ms_Singleton = 0;

// instance count
template<class T>
class InstanceCount : public virtual NoCopy
{
public:
    InstanceCount()
    {
        ms_count++;
    }
    virtual ~InstanceCount()
    {
        ms_count--;
    }
    static size_t getInstanceCount()
    {
        return ms_count;
    }
protected:
    static size_t ms_count;
};

template<class T>
size_t InstanceCount<T>::ms_count = 0;

class RefCountedObj : public NoCopy
{
public:
    RefCountedObj() : count(0)
    {
    }
    virtual ~RefCountedObj()
    {
        assert(count == 0);
    }
    void incRef()
    {
        count++;
    }
    void decRef()
    {
        count--;
    }
protected:
    size_t count;
};

template<class T>
class Ref
{
public:
    Ref() : obj(0)
    {
    }
    Ref(const Ref &o)
    {
        this->obj = o.obj;
        if (this->obj)
        {
            this->obj->incRef();
        }
    }
    Ref &operator = (const Ref &o)
    {
        release();
        this->obj = o.obj;
        if (this->obj)
        {
            this->obj->incRef();
        }
        return *this;
    }
    virtual ~Ref()
    {
        release();
    }
    void operator = (T *obj)
    {
        release();
#ifdef _CPPRTTI
        this->obj = dynamic_cast<RefCountedObj *>(obj);
#else
        this->obj = (T *)obj;
#endif
        if (this->obj)
        {
            this->obj->incRef();
        }
    }
    void release()
    {
        if (this->obj)
        {
            this->obj->decRef();
            this->obj = 0;
        }
    }
    T *getObj() const
    {
#ifdef _CPPRTTI
        return dynamic_cast<T *>(obj);
#else
        return (T *)obj;
#endif
    }
protected:
    RefCountedObj *obj;
};

inline float sign(float x)
{
    return (x < 0.0f)? -1.0f : 1.0f;
}

inline float frand(float x = 1.0f)
{
    return (rand() / (float) RAND_MAX) * x;
}

inline float Pi()
{
    static const float pi = atan(1.0f) * 4.0f;

    return pi;
}

template<typename T>
inline T clamp(const T &a, const T &lo, const T &hi)
{
    assert(lo <= hi);
    if (a >= hi)
        return hi;
    else if (a <= lo)
        return lo;
    return a;
}

template<typename T>
inline T lerp(const T &a, const T &b, float fLerp)
{
    return a + (b - a) * fLerp;
}

inline Point2f SnapNearestInteger(const Point2f &f)
{
    Point2f t = f;
    t.x = floor(t.x + 0.5f);
    t.y = floor(t.y + 0.5f);
    return t;
}

extern bool LineCrossPoint(const Point2f &va, const Point2f &vb, const Point2f &vc, const Point2f &vd, Point2f &out);
extern bool LineCross(const Point2f &va, const Point2f &vb, const Point2f &vc, const Point2f &vd);
extern float DistancePointToEdge(const Point2f &p, const Point2f &a, const Point2f &b);

#endif//UTILS_H
