//
//  IATriangle.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_TRIANGLE_H
#define IA_TRIANGLE_H


#include "IAVector3d.h"

#include <vector>


class IAVertex;
class IAEdge;


class IATriangle
{
public:
    IATriangle();
    bool validNormal() { return pNNormal==1; }
    void rotateVertices();
    void print();
    int pointsBelowZ(double z);

    IAVertex *pVertex[3] = { nullptr, nullptr, nullptr };
    IAEdge *pEdge[3] = { nullptr, nullptr, nullptr };
    IAVector3d pNormal;
    int pNNormal = 0;
    bool pUsed = false;
    bool pPatched = false;
};

typedef IATriangle *IATrianglePtr;
typedef std::vector<IATriangle*> IATriangleList;


#endif /* IA_TRIANGLE_H */


