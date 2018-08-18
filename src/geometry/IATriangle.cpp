//
//  IATriangle.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IATriangle.h"

#include "IAVertex.h"


/**
 Create an emoty triangle.
 */
IATriangle::IATriangle()
{
}


/**
 Change the numbering of vertices and edges in this triangle.
 */
void IATriangle::rotateVertices()
{
    IAVertex *v = pVertex[0];
    pVertex[0] = pVertex[1];
    pVertex[1] = pVertex[2];
    pVertex[2] = v;
    IAEdge *e = pEdge[0];
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
    pVertex[0]->print();
    pVertex[1]->print();
    pVertex[2]->print();
}


/**
 Return the number of points in this triangle that lay below a Z threshold.
 */
int IATriangle::pointsBelowZGlobal(double zMin)
{
    double z0 = pVertex[0]->pGlobalPosition.z();
    double z1 = pVertex[1]->pGlobalPosition.z();
    double z2 = pVertex[2]->pGlobalPosition.z();
    int n = (z0<zMin) + (z1<zMin) + (z2<zMin);
    return n;
}

