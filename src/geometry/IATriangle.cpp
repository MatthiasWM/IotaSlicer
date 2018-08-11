//
//  IATriangle.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IATriangle.h"

#include "IAVertex.h"


IATriangle::IATriangle()
{
}

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

void IATriangle::print()
{
    printf("Face: \n");
    pVertex[0]->print();
    pVertex[1]->print();
    pVertex[2]->print();
}

int IATriangle::pointsBelowZ(double zMin)
{
    double z0 = pVertex[0]->pPosition.z();
    double z1 = pVertex[1]->pPosition.z();
    double z2 = pVertex[2]->pPosition.z();
    int n = (z0<zMin) + (z1<zMin) + (z2<zMin);
    return n;
}

