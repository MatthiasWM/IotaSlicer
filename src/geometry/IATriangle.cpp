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
//:   pMesh(m)
{
}


/**
 * Check if a triangle in global spaces intersects with the z plane.
 *
 * \param zMin given height
 *
 * \return false, if all vertices of the triangle is entirely below z,
 *      or if all vertices are equal of above z.
 */
bool IATriangle::crossesZGlobal(double zMin)
{
    double z0 = vertex(0)->pGlobalPosition.z();
    double z1 = vertex(1)->pGlobalPosition.z();
    double z2 = vertex(2)->pGlobalPosition.z();
    int nBelow = (z0<zMin) + (z1<zMin) + (z2<zMin);
    return (nBelow==1 || nBelow==2);
}

