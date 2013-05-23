//------------------------------------------------------------------------------------------------ 
////////////////////////////////////////////////////////////////////////////////////////////////// 
// 
// File          : Contact.cpp
// 
// Created by    : OR - 07/02/2004 12:42:27
// 
// Copyright (C) : 2004 Rebellion, All Rights Reserved.
// 
////////////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Description   : 
// --------------- 
// 
// 
////////////////////////////////////////////////////////////////////////////////////////////////// 
//------------------------------------------------------------------------------------------------ 


#include <caPoint2d.h>
#include "verlet/verlet.h"
using cAni::Point2f;

class CMaterial
{
public:
    CMaterial(float fCoF = 0.1f, float fCoR = 0.3f, float fSep=0.5f)
        : m_fCoF(fCoF)
        , m_fCoR(fCoR)
        , m_fSep(fSep)
    {}

    void SetSeparation (float fSep) { m_fSep = fSep; }
    void SetFriction   (float fCoF) { m_fCoF = fCoF; }
    void SetRestitution(float fCoR) { m_fCoR = fCoR; }

    float GetSeparation () const { return m_fSep; }
    float GetFriction   () const { return m_fCoF; }
    float GetRestitution() const { return m_fCoR; }

private:
    float m_fCoF, m_fCoR, m_fSep;
};

// HACK : use a shared material for all objects
CMaterial	 s_xContactMaterial;

namespace verlet
{
    void Contact::ResolveOverlap()
    {
        if (!m_pxBodies[0] || !m_pxBodies[1])
            return;
        if (m_pxBodies[0]->isStaticBody() && m_pxBodies[1]->isStaticBody())
            return;

        float fRelaxation = s_xContactMaterial.GetSeparation();

        float m0 = m_pxBodies[0]->getInvMass();
        float m1 = m_pxBodies[1]->getInvMass();
        float m  = m0 + m1;

        Point2f D = m_xContacts[1] - m_xContacts[0];
        D *= fRelaxation;

        Point2f D0 = Point2f(0.0f, 0.0f);
        Point2f D1 = Point2f(0.0f, 0.0f);

        if (m0 > 0.0f)
        {
            D0 = D * (m0 / m);
            //if (m_pxBodies[0]->isStaticBody())
            //    m_pxBodies[1]->setPosition(m_pxBodies[1]->getPosition() - D1);
            //else
                m_pxBodies[0]->setPosition(m_pxBodies[0]->getPosition() + D0);
        }
        if (m1 > 0.0f) 
        {
            D1 = D * -(m1 / m);
            //if (m_pxBodies[1]->isStaticBody())
            //    m_pxBodies[0]->setPosition(m_pxBodies[0]->getPosition() - D1);
            //else
                m_pxBodies[1]->setPosition(m_pxBodies[1]->getPosition() + D1);
        }
    }

    void Contact::ResolveCollision()
    {
        if (!m_pxBodies[0] || !m_pxBodies[1])
            return;

        //------------------------------------------------------------------------------------------------------
        // parameters
        //------------------------------------------------------------------------------------------------------
        Point2f C0    = m_xContacts[0];
        Point2f C1    = m_xContacts[1];
        Point2f Ncoll = C1 - C0;
        // assert(Ncoll.x != 0 || Ncoll.y != 0);
        Ncoll.Normalize();

        float m0 = m_pxBodies[0]->getInvMass();
        float m1 = m_pxBodies[1]->getInvMass();
        float i0 = m_pxBodies[0]->getInvInertia();
        float i1 = m_pxBodies[1]->getInvInertia();

        const Point2f& P0   = m_pxBodies[0]->getPosition();
        const Point2f& P1   = m_pxBodies[1]->getPosition();
        const Point2f& V0   = m_pxBodies[0]->getVelocity();
        const Point2f& V1   = m_pxBodies[1]->getVelocity();
        float  w0   = m_pxBodies[0]->getAngVelocity();
        float  w1   = m_pxBodies[1]->getAngVelocity();

        //------------------------------------------------------------------------------------------------------
        // pre-computations
        //------------------------------------------------------------------------------------------------------
        Point2f R0    = C0 - P0;
        Point2f R1    = C1 - P1;
        Point2f T0    = Point2f(-R0.y, R0.x);
        Point2f T1    = Point2f(-R1.y, R1.x);
        Point2f VP0   = V0 + T0 * w0; // point velocity
        Point2f VP1   = V1 + T1 * w1; // point velocity

        //------------------------------------------------------------------------------------------------------
        // impact velocity
        //------------------------------------------------------------------------------------------------------
        Point2f Vcoll = VP0 - VP1;
        float  vn	 = Vcoll * Ncoll;
        Point2f Vn	 = Ncoll * vn;
        Point2f Vt	 = Vcoll - Vn;

        if (vn > 0.0f) // separation
            return;

        if (Vt * Vt < 0.0001f)
            Vt = Point2f(0.0f, 0.0f);

        // float  vt = Vt.Length();
        Vt.Normalize();


        //------------------------------------------------------------------------------------------------------
        // compute impulse (frction and restitution).
        // ------------------------------------------
        //
        //									-(1+Cor)(Vel.norm)
        //			j =  ------------------------------------------------------------
        //			     [1/Ma + 1/Mb] + [Ia' * (ra x norm)²] + [Ib' * (rb x norm)²]
        //------------------------------------------------------------------------------------------------------
        Point2f J;
        Point2f Jt(0.0f, 0.0f);
        Point2f Jn(0.0f, 0.0f);

        float fCoR  = s_xContactMaterial.GetRestitution();
        float fCoF  = s_xContactMaterial.GetFriction();

        float t0 = (R0 ^ Ncoll) * (R0 ^ Ncoll) * i0;
        float t1 = (R1 ^ Ncoll) * (R1 ^ Ncoll) * i1;
        float m  = m0 + m1;

        float denom = m + t0 + t1;

        float jn = vn / denom;

        Jn = Ncoll * (-(1.0f + fCoR) * jn);

        //if (dbg_UseFriction)
        Jt = Vt * (fCoF * jn);

        J = Jn + Jt;

        //------------------------------------------------------------------------------------------------------
        // changes in momentum
        //------------------------------------------------------------------------------------------------------
        Point2f dV0 = J * m0;
        Point2f dV1 =-J * m1;

        float dw0 = (R0 ^ J) * i0;
        float dw1 =-(R1 ^ J) * i1;

        //------------------------------------------------------------------------------------------------------
        // apply changes in momentum
        //------------------------------------------------------------------------------------------------------
        if (!m_pxBodies[0]->isStaticBody())
        {
            if (m0 > 0.0f)
            {
                m_pxBodies[0]->setVelocity(V0 + dV0);
            }
            if (m0 > 0.0f)
            {
                m_pxBodies[0]->setAngVelocity(w0 + dw0);
            }
        }
        if (!m_pxBodies[1]->isStaticBody())
        {
            if (m1 > 0.0f)
            {
                m_pxBodies[1]->setVelocity(V1 + dV1);
            }
            if (m1 > 0.0f)
            {
                m_pxBodies[1]->setAngVelocity(w1 + dw1);
            }
        }
        //	Render();
        return;
/*
        //------------------------------------------------------------------------------------------------------
        // Check for static frcition
        //------------------------------------------------------------------------------------------------------
        float fRestingContactVelocity = 1.0f;

        if (-vn < fRestingContactVelocity)
        {
            if (vt < fRestingContactVelocity * fCoF)
            {
                //------------------------------------------------------------------------------------------------------
                // Cancel tangential velocity on the two bodies, so they stick
                //------------------------------------------------------------------------------------------------------
                Point2f dV = V1 - V0;
                dV -= Ncoll * (dV * Ncoll);
                if (m0 > 0.0f) V0 += dV * (m0 / m + 0.01f);
                if (m1 > 0.0f) V1 -= dV * (m1 / m + 0.01f);
            }
        }
        */
    }

}