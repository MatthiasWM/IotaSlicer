//
//  IATriangle.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IATriangle.h"

#include "IAVertex.h"

#include <stdio.h>


/**
 Create an emoty triangle.
 */
IATriangle::IATriangle(IAMesh *m)
:   pMesh(m)
{
}


/**
 Change the numbering of vertices and edges in this triangle.
 */
void IATriangle::rotateVertices()
{
    IAHalfEdge *e = pEdge[0];
    pEdge[0] = pEdge[1];
    pEdge[1] = pEdge[2];
    pEdge[2] = e;
}


/**
 Print basic information about this triangle.
 */
void IATriangle::print()
{
    printf("Face: \n");
    vertex(0)->print();
    vertex(1)->print();
    vertex(2)->print();
}


/**
 Return the number of points in this triangle that lay below a Z threshold.
 */
int IATriangle::pointsBelowZGlobal(double zMin)
{
    double z0 = vertex(0)->pGlobalPosition.z();
    double z1 = vertex(1)->pGlobalPosition.z();
    double z2 = vertex(2)->pGlobalPosition.z();
    int n = (z0<zMin) + (z1<zMin) + (z2<zMin);
    return n;
}

