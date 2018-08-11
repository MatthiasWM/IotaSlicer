//
//  IAEdge.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAEdge.h"

#include "IAVertex.h"
#include "IATriangle.h"


IAEdge::IAEdge()
{
}

IAVertex *IAEdge::vertex(int i, IATriangle *f)
{
    if (pFace[0]==f) {
        return pVertex[i];
    } else if (pFace[1]==f) {
        return pVertex[1-i];
    } else {
        puts("ERROR: vertex() - this edge is not associated with this face!");
        return 0L;
    }
}

IAVertex *IAEdge::findZ(double zMin)
{
    IAVertex *v0 = pVertex[0], *v1 = pVertex[1];
    IAVector3d vd0(v0->pPosition);
    bool retVec = false;
    vd0 -= v1->pPosition;
    double dzo = vd0.z(), dzn = zMin-v1->pPosition.z();
    double m = dzn/dzo;  // TODO: division by zero should not be possible...
    if (m>=0.0 && m<=1) retVec = true;
    if (retVec) {
        // calculate the coordinate at zMin
        vd0 *= m;
        vd0 += v1->pPosition;
        // calculate the texture coordinate at zMin
        IAVector3d vt0(v0->pTex);
        vt0 -= v1->pTex;
        vt0 *= m;
        vt0 += v1->pTex;
        IAVertex *v2 = new IAVertex();
        v2->pPosition = vd0;
        v2->pTex = vt0;
        return v2;
    } else {
        return 0L;
    }
}

IATriangle *IAEdge::otherFace(IATriangle *f)
{
    if (pFace[0]==f) {
        return pFace[1];
    } else if (pFace[1]==f) {
        return pFace[0];
    } else {
        puts("ERROR: otherFace() - this edge is not associated with this face!");
        return 0L;
    }
}

int IAEdge::indexIn(IATriangle *f)
{
    if (f->pEdge[0]==this) return 0;
    if (f->pEdge[1]==this) return 1;
    if (f->pEdge[2]==this) return 2;
    puts("ERROR: indexIn() - this edge was not found with this face!");
    return -1;
}

int IAEdge::nFaces()
{
    int n = 0;
    if (pFace[0]) n++;
    if (pFace[1]) n++;
    return n;
}

