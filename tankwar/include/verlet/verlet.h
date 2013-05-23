#ifndef VERLET_H
#define VERLET_H

#include <cassert>
#include <vector>
#include <caPoint2d.h>
#include <caRect.h>
#include "common/utils.h"
namespace verlet
{
    using std::vector;
    using cAni::Point2f;
    using cAni::Rectf;

    class iShape;
    class iBody;
    class LinConstraint
    {
    public:
        void Update();
    protected:
        iBody *m_pxBodyA, *m_pxBodyB;
        float m_fRestLength;
    };
    class Contact
    {
    public:
        enum { eMaxContacts = 2 };

        iBody *m_pxBodies[2];
        Point2f m_xContacts[eMaxContacts];

        Contact()
        {
            Reset();
        }

        Contact(const Point2f &PA, const Point2f &PB, iBody *pxBodyA, iBody *pxBodyB)
        {
            //assert(PA.x != PB.x || PA.y != PB.y);
            m_pxBodies[0]  = pxBodyA;
            m_pxBodies[1]  = pxBodyB;
            m_xContacts[0] = PA;
            m_xContacts[1] = PB;
        }

        void Reset()
        {
            m_pxBodies[0] = m_pxBodies[1] = 0;
        }

        iBody *GetBody(int i)
        {
            return m_pxBodies[i];
        }
        Point2f &GetContact(int i)
        {
            return m_xContacts[i];
        }

        void ResolveCollision();
        void ResolveOverlap();
    };
    class Body;
    class StaticBody;
    class iBody
    {
    public:
        void SetShape(iShape *shape)
        {
            this->shape = shape;
        }
        iShape *GetShape()
        {
            return this->shape;
        }
        const iShape *GetShape() const
        {
            return this->shape;
        }
        float getOrientation() const
        {
            return orientation;
        }
        void setOrientation(float ori)
        {
            orientation = ori;
        }
        const Point2f &getPosition() const
        {
            return pos;
        }
        void setPosition(const Point2f &pos)
        {
            this->pos = pos;
        }
        virtual void Reset(const Point2f& xPosition, float fOrientation, float fMass)
        {
            pos = xPosition;
            orientation = fOrientation;
            fMass;
        }
        virtual size_t Collide(iBody& body, float dt, Contact *pxContacts, size_t numMaxContacts);
        virtual Point2f GetPointVelocity(const Point2f& P, float dt)
        {
            P, dt;
            return Point2f();
        }
        virtual void AddForce(const Point2f& F)
        {
            F;
        }
        virtual void AddTorque(float T)
        {
            T;
        }
        virtual void AddForce(const Point2f& F, const Point2f& P)
        {
            F, P;
        }
        virtual const Point2f &getVelocity() const
        {
            static Point2f zero;
            return zero;
        }
        virtual const float getAngVelocity() const
        {
            return 0.f;
        }
        virtual void Update(float dt)
        {
            dt;
        }
        virtual float getMass() const
        {
            return 0.0f;
        }
        virtual float getInvMass() const
        {
            return 0.0f;
        }
        virtual float getInvInertia() const
        {
            return 0.0f;
        }
        virtual void setAngVelocity(const float &av)
        {
            av;
        }
        virtual void setVelocity(const Point2f &vel)
        {
            vel;
        }
        virtual bool isStaticBody() const = 0;
    protected:
        iBody() : shape(0), orientation(0.0f)
        {
        }
        iShape *shape;
        Point2f pos;
        float orientation;
    };
    class Body : public iBody
    {
    public:
        void Reset(const Point2f& xPosition, float fOrientation, float fMass);

        void AddForce(const Point2f& F)
        {
            assert(F.Length() < 1e2);
            netForce += F;
        }
        void AddTorque(float T)
        {
            netTorque += T;
        }
        void AddForce(const Point2f& F, const Point2f& P)
        {
            assert(F.Length() < 1e2);
            netForce += F;
            netTorque += (P - pos) ^ F;
        }
        Point2f GetPointVelocity(const Point2f& P, float dt)
        {
            dt;

            Point2f D(P - pos);
            Point2f L(-D.y, D.x);

            return velocity + L * angVelocity;
        }
        
        void Update(float dt);
        const Point2f &getVelocity() const
        {
            return velocity;
        }
        void setVelocity(const Point2f &vel)
        {
            assert((vel - velocity).Length() < 1e5);
            velocity = vel;
        }
        const float getAngVelocity() const
        {
            return angVelocity;
        }
        void setAngVelocity(const float &av)
        {
            angVelocity = av;
        }
        float getMass() const
        {
            return mass;
        }
        float getInvMass() const
        {
            return invMass;
        }
        float getInertia() const
        {
            return inertia;
        }
        float getInvInertia() const
        {
            return invInertia;
        }
        virtual bool isStaticBody() const
        {
            return false;
        }
    protected:
        vector<LinConstraint> constraints;

        float mass;
        float invMass;

        Point2f velocity;
        float angVelocity; // angular velocity

        Point2f netForce;
        float netTorque;

        float inertia;
        float invInertia;
    };
    class StaticBody : public iBody
    {
    public:
        void Reset(const Point2f& xPosition, float fOrientation);
        virtual bool isStaticBody() const
        {
            return true;
        }
    };
    class BoxShape;
    class CircleShape;

#define DEFINE_SHAPE_POLYMOROHIC \
    virtual size_t collide(iShape &shape, float dt, Contact *pxContacts, size_t numMaxContacts) = 0; \
    virtual size_t collide(BoxShape &shape, float dt, Contact *pxContacts, size_t numMaxContacts) = 0; \
    virtual size_t collide(CircleShape &shape, float dt, Contact *pxContacts, size_t numMaxContacts) = 0;
    // end of define

#define IMPLEMENT_A_SHAPE_POLYMOROHIC(type) \
    virtual size_t collide(type &shape, float dt, Contact *pxContacts, size_t numMaxContacts) \
    { return verlet::collide(*this, shape, dt, pxContacts, numMaxContacts); }
    // end of define 

#define IMPLEMENT_SHAPE_POLYMOROHIC \
    IMPLEMENT_A_SHAPE_POLYMOROHIC(BoxShape) \
    IMPLEMENT_A_SHAPE_POLYMOROHIC(CircleShape) \
    virtual size_t collide(iShape &shape, float dt, Contact *pxContacts, size_t numMaxContacts) \
    { return shape.collide(*this, dt, pxContacts, numMaxContacts); }
    // end of define

    class iShape : public NoCopy
    {
    public:
        iShape(iBody &_body) : body(_body)
        {
        }
        virtual float GetInertiaMoment() const = 0;
        DEFINE_SHAPE_POLYMOROHIC;
        virtual void update(){}
        virtual Rectf GetBoundingBox() const = 0;

        iBody &body; // attached body
//        Point2f pos;
//        Point2f dir;
    };

    size_t collide(BoxShape &box1, BoxShape &box2, float dt, Contact *pxContacts, size_t numMaxContacts);
    size_t collide(CircleShape &circle, BoxShape &box, float dt, Contact *pxContacts, size_t numMaxContacts);
    inline size_t collide(BoxShape &box, CircleShape &circle, float dt, Contact *pxContacts, size_t numMaxContacts)
    {
        return collide(circle, box, dt, pxContacts, numMaxContacts);
    }
    size_t collide(CircleShape &circle1, CircleShape &circle2, float dt, Contact *pxContacts, size_t numMaxContacts);
    class BoxShape : public iShape
    {
    public:
        BoxShape(iBody &_body, const Point2f &_extents) : iShape(_body), extents(_extents)
        {

        }
        virtual float GetInertiaMoment() const
        {
            float fWidth  = extents.x * 2.0f;
            float fHeight = extents.y * 2.0f;

            return 1.0f / 12.0f * (fWidth * fWidth + fHeight * fHeight);
        }
        IMPLEMENT_SHAPE_POLYMOROHIC;
        virtual void update()
        {
            dir[0] = Point2f(cos(body.getOrientation()), sin(body.getOrientation()));
            dir[1] = Point2f(-dir[0].y, dir[0].x);
        }
        Rectf GetBoundingBox() const
        {
            Point2f p1 = dir[0] * extents.x;
            if (p1.x < 0)
                p1.x = -p1.x;
            if (p1.y < 0)
                p1.y = -p1.y;
            Point2f p2 = dir[1] * extents.y;
            if (p2.x < 0)
                p2.x = -p2.x;
            if (p2.y < 0)
                p2.y = -p2.y;
            Rectf r;
            r.rightBottom = p1 + p2;
            r.leftTop = - r.rightBottom;
            return r;
        }
        const Point2f &GetDir(int id) const
        {
            assert(id == 0 || id == 1);
            return dir[id];
        }
        const Point2f &GetExtents() const
        {
            return extents;
        }
        size_t FindSupportPoints(const Point2f& xAxis, Point2f* pxPoints) const
        {
            float f0 = dir[0] * xAxis;
            float f1 = dir[1] * xAxis;
            float s0 = sign(f0);
            float s1 = sign(f1);

            size_t numPoints = 0;
            pxPoints[numPoints++] = body.getPosition() - 
                dir[0] * extents.x * s0 - 
                dir[1] * extents.y * s1;

            if (fabs(f0) < 0.001f)
            {
                pxPoints[numPoints++] = pxPoints[0] + dir[0] * extents.x * s0 * 2.0f;
            }
            else if (fabs(f1) < 0.001f)
            {
                pxPoints[numPoints++] = pxPoints[0] + dir[1] * extents.y * s1 * 2.0f;
            }

            return numPoints;
        }
        void GetInterval(const Point2f& xAxis, float& min,  float& max) const
        {
            float p = body.getPosition() * xAxis;
            float r = fabs(dir[0] * xAxis) * extents.x + fabs(dir[1] * xAxis) * extents.y;

            min = p - r;
            max = p + r;
        }
    protected:
        Point2f extents;
        Point2f dir[2];
    };
    class CircleShape : public iShape
    {
    public:
        CircleShape(iBody &_body, float _radius) : iShape(_body), radius(_radius)
        {
        }

        virtual float GetInertiaMoment() const
        {
            return 2.0f / 5.0f * (radius * radius);
        }
        IMPLEMENT_SHAPE_POLYMOROHIC;
        Rectf GetBoundingBox() const
        {
            Rectf r;
            r.rightBottom = Point2f(radius, radius);
            r.leftTop = -r.rightBottom;
            return r;
        }
        float GetRadius() const
        {
            return radius;
        }
    protected:
        float radius;
    };
    /*
    class PolygonShape : public iShape
    {
    protected:
        vector<Point2f> vertices;
    };
    */
}

#endif//VERLET_H