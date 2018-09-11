//
//  IAEdge.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAEdge.h"

#include "IAMesh.h"
#include "IAVertex.h"
#include "IATriangle.h"

#include <stdio.h>


/**
 Create an edge with no connections.
 */
IAEdge::IAEdge()
{
}


//============================================================================//


/**
 * Create a half-edge that will be part of a triangle, pointing at a vertex.
 */
IAHalfEdge::IAHalfEdge(IATriangle *t, IAVertex *v)
:   pTriangle( t ),
    pVertex( v )
{
}


/**
 Find the intersection of this edge with a give Z plane.
 \return a vector on this edge with interpolated texture coordinates, or null
 if this edge does not cross the Z plane.
 */
IAVertex *IAHalfEdge::findZGlobal(double zMin)
{
    IAVertex *v0 = vertex(), *v1 = next()->vertex();
    IAVector3d vd0(v0->pGlobalPosition);
    bool retVec = false;
    vd0 -= v1->pGlobalPosition;
    double dzo = vd0.z(), dzn = zMin-v1->pGlobalPosition.z();
    /** \todo division by zero should not be possible...
     we did avoid it by the z-offset hack, but, ugh!
     */
    double m = dzn/dzo;
    if (m>=0.0 && m<=1) retVec = true;
    if (retVec) {
        // calculate the coordinate at zMin
        vd0 *= m;
        vd0 += v1->pGlobalPosition;
        // calculate the texture coordinate at zMin
        IAVector3d vt0(v0->pTex);
        vt0 -= v1->pTex;
        vt0 *= m;
        vt0 += v1->pTex;
        IAVertex *v2 = new IAVertex();
        v2->pGlobalPosition = vd0;
        v2->pLocalPosition = vd0;
        v2->pTex = vt0;
        v2->pNormal = (v0->pNormal*m + v1->pNormal*(1.0-m));
        v2->pNormal.normalize();
        return v2;
    } else {
        return nullptr;
    }
}


/**
 * Find the next edge (ccw) in this fan that has no twin.
 *
 * Walk the fan until we either find an edge without twin, or we find this edge
 * again, which indicates a complete fan. This is normally called when this
 * edge has no twin either.
 *
 * \return the next edge in the fan that has no twin; the returned half-edge
 *      next->vertex is the same as \e this vertex.
 * \return nullptr if the fan is complete, like a cocktail umbrella.
 */
IAHalfEdge *IAHalfEdge::findPrevSingleEdgeInFan()
{
    IAHalfEdge *e = prev();
    for (;;) {
        if (!e->twin()) return e;
        if (e->twin()==this) return nullptr;
        e = e->twin()->prev();
        // if this loops endlessly, the mesh is seriously broken
    }
}


/**
 * Find the prev edge (cw) in this fan that has no twin.
 *
 * Walk the fan until we either find an edge without twin, or we find this edge
 * again, which indicates a complete fan. This is normally called when this
 * edge has no twin either.
 *
 * \return the prev edge in the fan that has no twin; the returned half-edge
 *      vertex is the same as this vertex. Can return \e this.
 * \return nullptr if the fan is complete, like a cocktail umbrella.
 */
IAHalfEdge *IAHalfEdge::findNextSingleEdgeInFan()
{
    if (!twin()) return this;
    IAHalfEdge *e = twin()->next();
    for (;;) {
        if (e==this) return nullptr;
        if (!e->twin()) return e;
        e = e->twin()->next();
        // if this loops endlessly, the mesh is seriously broken
    }
}





