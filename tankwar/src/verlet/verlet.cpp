#include "verlet/verlet.h"


namespace verlet
{
    size_t iBody::Collide(iBody& body, float dt, Contact *pxContacts, size_t numMaxContacts)
    {
        if (!shape || !body.shape || this->isStaticBody() && body.isStaticBody())
            return 0;
        assert(shape && body.shape);
        return shape->collide(*body.shape, dt, pxContacts, numMaxContacts);
    }
    void Body::Reset(const Point2f& xPosition, float fOrientation, float fMass)
    {
        this->pos           = xPosition;
        this->orientation   = fOrientation;
        this->mass          = fMass;
        this->invMass       = (fMass == 0.0f) ? 0.0f : 1.0f / fMass;
        this->inertia       = (fMass == 0.0f || !shape) ? 0.0f : mass * shape->GetInertiaMoment();
        this->invInertia    = (mass == 0.0f) ? 0.0f : 1.0f / inertia;
        this->velocity      = Point2f(0.0f, 0.0f);
        this->angVelocity   = 0.0f;
        this->netForce      = Point2f(0.0f, 0.0f);
        this->netTorque     = 0.0f;
        if (this->shape)
            this->shape->update();
    }
    void Body::Update(float dt)
    {
        //-------------------------------------------------------
        // Integrate position (verlet integration)
        //-------------------------------------------------------
        Point2f xAccel  = netForce * invMass;
        pos += velocity * dt;// + 0.5f * xAccel * dt*dt;
        orientation += angVelocity * dt;

        assert(xAccel.Length() < 1e3);
        //-------------------------------------------------------
        // Integrate velocity (implicit linear velocity)
        //-------------------------------------------------------
        velocity += xAccel * dt;
        angVelocity += netTorque * (invInertia * dt);

        //-------------------------------------------------------
        // clear forces
        //-------------------------------------------------------
        netForce = Point2f(0.0f, 0.0f);
        netTorque = 0.0f;

        if (shape)
            shape->update();
    }
    void StaticBody::Reset(const Point2f& xPosition, float fOrientation)
    {
        iBody::Reset(xPosition, fOrientation, 0.f);

        if (shape)
            shape->update();
    }
    bool ProjectPointOnSegment(const Point2f& P, const Point2f& E0, const Point2f& E1, Point2f& Q)
    {
        Point2f E = E1 - E0;
        Point2f D = P  - E0;
        float e2 = E * E;
        float de = D * E;

        if (de < 0.0f || de > e2)
            return false;

        Q = E0 + E * (de / e2);

        return true;
    }

    bool ConvertSupportPointsToContacts(const BoxShape& box1, const BoxShape& box2, 
        const Point2f& xAxis,
        Point2f* pxPoints0, size_t numP0, 
        Point2f* pxPoints1, size_t numP1,
        Contact* xContacts, size_t& iNumContacts)
    {
        iNumContacts = 0;

        Point2f xPerp(-xAxis.y, xAxis.x);

        float min0 = pxPoints0[0] * xPerp;
        float max0 = min0;
        float min1 =  pxPoints1[0] * xPerp;
        float max1 = min1;

        if (numP0 == 2)
        {
            max0 = pxPoints0[1] * xPerp;

            if (max0 < min0) 
            { 
                float c = min0;
                min0 = max0;
                max0 = c;

                Point2f T = pxPoints0[0];
                pxPoints0[0] = pxPoints0[1];
                pxPoints0[1] = T;
            }
        }

        if (numP1 == 2)
        {
            max1 = pxPoints1[1] * xPerp;

            if (max1 < min1) 
            { 
                float c = min1;
                min1 = max1;
                max1 = c;

                Point2f T = pxPoints1[0];
                pxPoints1[0] = pxPoints1[1];
                pxPoints1[1] = T;
            }
        }

        if (min0 > max1 || min1 > max0)
            return false;

        if (min0 > min1)
        {
            Point2f Pseg;
            if (ProjectPointOnSegment(pxPoints0[0], pxPoints1[0], pxPoints1[1], Pseg))
            {
                xContacts[iNumContacts++] = Contact(pxPoints0[0], Pseg, &box1.body, &box2.body);
            }
        }
        else
        {
            Point2f Pseg;
            if (ProjectPointOnSegment(pxPoints1[0], pxPoints0[0], pxPoints0[1], Pseg))
            {
                xContacts[iNumContacts++] = Contact(Pseg, pxPoints1[0], &box1.body, &box2.body);
            }
        }

        if (max0 != min0 && max1 != min1)
        {
            if (max0 < max1)
            {
                Point2f Pseg;
                if (ProjectPointOnSegment(pxPoints0[1], pxPoints1[0], pxPoints1[1], Pseg))
                {
                    xContacts[iNumContacts++] = Contact(pxPoints0[1], Pseg, &box1.body, &box2.body);
                }
            }
            else
            {
                Point2f Pseg;
                if (ProjectPointOnSegment(pxPoints1[1], pxPoints0[0], pxPoints0[1], Pseg))
                {
                    xContacts[iNumContacts++] = Contact(Pseg, pxPoints1[1], &box1.body, &box2.body);
                }
            }
        }
        return true;
    }

    bool FindCollisionPlane(const Point2f* xAxes, float* d, Point2f& Ncoll, float& dcoll)
    {
        float dmin = 1000000.0f;
        int imin = -1;

        for(int i = 0; i < 4; i ++)
        {
            float a = xAxes[i].Length();

            if (a < 0.000001f)
                continue;

            float depth = d[i] / a;

            if (depth < dmin)
            {
                dmin = depth;
                imin = i;
                Ncoll = xAxes[i] / a;
                dcoll = depth;
            }
        }

        return (imin != -1);
    }
    bool IntervalIntersect(const Point2f& xAxis, const BoxShape& box1, const BoxShape& box2, float& depth)
    {
        float min0, max0;
        float min1, max1;

        box1.GetInterval(xAxis, min0, max0);
        box2.GetInterval(xAxis, min1, max1);

        if (min0 > max1 || min1 > max0) return false;

        float d0 = max1 - min0;
        float d1 = max0 - min1;

        if (d0 < d1)
            depth = d0;
        else
            depth = d1;

        return true;
    }

    size_t collide(BoxShape &box1, BoxShape &box2, float dt, Contact *pxContacts, size_t numMaxContacts)
    {
        dt;

        Point2f xAxis [4];
        float  fDepth[4];

        xAxis[0] = box1.GetDir(0);
        if (!IntervalIntersect(xAxis[0], box1, box2, fDepth[0])) return 0;

        xAxis[1] = box1.GetDir(1);
        if (!IntervalIntersect(xAxis[1], box1, box2, fDepth[1])) return 0;

        xAxis[2] = box2.GetDir(0);
        if (!IntervalIntersect(xAxis[2], box1, box2, fDepth[2])) return 0;

        xAxis[3] = box2.GetDir(1);
        if (!IntervalIntersect(xAxis[3], box1, box2, fDepth[3])) return 0;

        float dcoll;
        Point2f Ncoll;
        if (!FindCollisionPlane(xAxis, fDepth, Ncoll, dcoll)) return 0;

        Point2f D = box1.body.getPosition() - box2.body.getPosition();

        if (D * Ncoll < 0.0f)
            Ncoll *= -1.0f;

        Point2f xPoints0[4];
        Point2f xPoints1[4];
        size_t numPoints0;
        size_t numPoints1;

        numPoints0 = box1.FindSupportPoints( Ncoll, xPoints0);
        numPoints1 = box2.FindSupportPoints(-Ncoll, xPoints1);

        if (!ConvertSupportPointsToContacts(box1, box2, Ncoll, xPoints0, numPoints0, xPoints1, numPoints1, pxContacts, numMaxContacts))
            return 0;
        return numMaxContacts;
    }
    size_t collide(CircleShape &circle, BoxShape &box, float dt, Contact *pxContacts, size_t numMaxContacts)
    {
        dt, pxContacts, numMaxContacts;
        const Point2f &p1 = circle.body.getPosition();
        const Point2f &p2 = box.body.getPosition();

        // work in the box's coordinate system
        const Point2f kDiff = p1 - p2;

        // compute squared distance and closest point on box
        float fSqrDistance = 0.0f, fDelta;
        Point2f kClosest(kDiff * box.GetDir(0), kDiff * box.GetDir(1));
        const Point2f &extents = box.GetExtents();
        if (kClosest.x < -extents.x)
        {
            fDelta = kClosest.x + extents.x;
            fSqrDistance += fDelta*fDelta;
            kClosest.x = -extents.x;
        }
        else if (kClosest.x > extents.x)
        {
            fDelta = kClosest.x - extents.x;
            fSqrDistance += fDelta*fDelta;
            kClosest.x = extents.x;
        }
        if (kClosest.y < -extents.y)
        {
            fDelta = kClosest.y + extents.y;
            fSqrDistance += fDelta*fDelta;
            kClosest.y = -extents.y;
        }
        else if (kClosest.y > extents.y)
        {
            fDelta = kClosest.y - extents.y;
            fSqrDistance += fDelta*fDelta;
            kClosest.y = extents.y;
        }
        if (fSqrDistance > circle.GetRadius() * circle.GetRadius())
        {
            return 0;
        }
        Point2f d = p2 + kClosest - p1;
        d.Normalize();

        pxContacts[0] = Contact(p1 + d * circle.GetRadius(), p2 + kClosest, &circle.body, &box.body);
        return 1;
    }
    size_t collide(CircleShape &circle1, CircleShape &circle2, float dt, Contact *pxContacts, size_t numMaxContacts)
    {
        dt, numMaxContacts;

        float rR = circle1.GetRadius() + circle2.GetRadius();
        const Point2f &p1 = circle1.body.getPosition();
        const Point2f &p2 = circle2.body.getPosition();
        Point2f d = p2 - p1;
        float len = d.Length();
        if (len > rR)
        {
            return 0;
        }
        d.Normalize();
        
        pxContacts[0] = Contact(p1 + d * circle1.GetRadius(), p1 + d * (len - circle1.GetRadius()), &circle1.body, &circle2.body);
        return 1;
    }

}