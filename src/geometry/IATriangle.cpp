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
 * Check if a triangle in global spaces intersects with the z plane.
 *
 * \param z given height
 * \return false, if all vertices of the triangle is entirely below z,
 *      or if all vertices are equal of above z.
 */
bool IATriangle::crossesZGlobal(double z)
{
    int nBelow = pointsBelowZGlobal(z);
    return (nBelow==1 || nBelow==2);
}


/**
 * Return the number of vertices in this triangle that lay below a Z threshold.
 *
 * Vertices that are on the z plane are not counted.
 */
int IATriangle::pointsBelowZGlobal(double zMin)
{
    double z0 = vertex(0)->pGlobalPosition.z();
    double z1 = vertex(1)->pGlobalPosition.z();
    double z2 = vertex(2)->pGlobalPosition.z();
    int n = (z0<zMin) + (z1<zMin) + (z2<zMin);
    return n;
}

