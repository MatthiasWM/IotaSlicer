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
int IAEdge::indexIn(IATriangle *f)
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
int IAEdge::nTriangle()
{
    int n = 0;
    if (pTriangle[0]) n++;
    if (pTriangle[1]) n++;
    return n;
}


//============================================================================//


IAHalfEdge::IAHalfEdge(IATriangle *t)
:   pTriangle(t)
{
}


//============================================================================//


IAHalfEdgeList::IAHalfEdgeList(IAMesh *m)
:   pMesh(m)
{
}


IAHalfEdgeList::~IAHalfEdgeList()
{
}





