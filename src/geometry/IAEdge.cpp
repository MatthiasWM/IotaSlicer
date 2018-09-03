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


/**
 Return the vertex, that connects to the given triangle.
 */
IAVertex *IAEdge::vertex(int i, IATriangle *f)
{
    if (pTriangle[0]==f) {
        return pVertex[i];
    } else if (pTriangle[1]==f) {
        return pVertex[1-i];
    } else {
        puts("ERROR: vertex() - this edge is not associated with this face!");
        return 0L;
    }
}


/**
 Find the intersection of this edge with a give Z plane.
 \return a vector on this edge with interpolated texture coordinates, or null
         if this edge does not cross the Z plane.
 */
IAVertex *IAEdge::findZGlobal(double zMin)
{
    IAVertex *v0 = pVertex[0], *v1 = pVertex[1];
    IAVector3d vd0(v0->pGlobalPosition);
    bool retVec = false;
    vd0 -= v1->pGlobalPosition;
    double dzo = vd0.z(), dzn = zMin-v1->pGlobalPosition.z();
    double m = dzn/dzo;  // FIXME: division by zero should not be possible...
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
 Return the other face of this edge, not the give one.
 \return null, there is no other face, or if the given face is not part of
         this edge.
 */
IATriangle *IAEdge::otherTriangle(IATriangle *f)
{
    if (pTriangle[0]==f) {
        return pTriangle[1];
    } else if (pTriangle[1]==f) {
        return pTriangle[0];
    } else {
        puts("ERROR: otherTriangle() - this edge is not associated with this face!");
        return 0L;
    }
}


/**
 Return the index of this edge in the edge list of the given face.
 \return 0, 1, or 2, or -1 if the face is not connected to this edge.
 */
//int IAEdge::indexIn(IATriangle *f)
//{
//    if (f->pEdge[0]==this) return 0;
//    if (f->pEdge[1]==this) return 1;
//    if (f->pEdge[2]==this) return 2;
//    puts("ERROR: indexIn() - this edge was not found with this face!");
//    return -1;
//}


/**
 Return the number of faces connected to this edge.
 */
int IAEdge::nTriangle()
{
    int n = 0;
    if (pTriangle[0]) n++;
    if (pTriangle[1]) n++;
    return n;
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
 Return the vertex, that connects to the given triangle.
 */
IAVertex *IAHalfEdge::vertex(int i, IATriangle *f)
{
    if (triangle()==f) {
        return vertex(i);
    } else if (twin() && twin()->triangle()==f) {
        return vertex(1-i);
    } else {
        puts("ERROR: vertex() - this edge is not associated with this face!");
        assert(0);
        return nullptr;
    }
}


/**
 Find the intersection of this edge with a give Z plane.
 \return a vector on this edge with interpolated texture coordinates, or null
 if this edge does not cross the Z plane.
 */
IAVertex *IAHalfEdge::findZGlobal(double zMin)
{
    IAVertex *v0 = vertex(0), *v1 = vertex(1);
    IAVector3d vd0(v0->pGlobalPosition);
    bool retVec = false;
    vd0 -= v1->pGlobalPosition;
    double dzo = vd0.z(), dzn = zMin-v1->pGlobalPosition.z();
    double m = dzn/dzo;  // FIXME: division by zero should not be possible...
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
 Return the other face of this edge, not the give one.
 \return null, there is no other face, or if the given face is not part of
 this edge.
 */
IATriangle *IAHalfEdge::otherTriangle(IATriangle *f)
{
    if (triangle()==f) {
        return triangle(1);
    } else if (triangle(1)==f) {
        return triangle();
    } else {
        puts("ERROR: otherTriangle() - this edge is not associated with this face!");
        assert(0);
        return 0L;
    }
}


/**
 Return the index of this edge in the edge list of the given face.
 \return 0, 1, or 2, or -1 if the face is not connected to this edge.
 */
int IAHalfEdge::indexIn(IATriangle *f)
{
    if (f->pEdge[0]==this) return 0;
    if (f->pEdge[1]==this) return 1;
    if (f->pEdge[2]==this) return 2;
    puts("ERROR: indexIn() - this edge was not found with this face!");
    return -1;
}


/**
 Return the number of faces connected to this edge.
 */
int IAHalfEdge::nTriangle()
{
    int n = 1;
    if (twin()) n = 2;
    return n;
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





