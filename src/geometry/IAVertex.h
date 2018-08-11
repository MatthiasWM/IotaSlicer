//
//  IAVertex.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_VERTEX_H
#define IA_VERTEX_H

#include "IAVector3d.h"

#include <vector>


class IAVertex
{
public:
    IAVertex();
    IAVertex(const IAVertex*);
    bool validNormal() { return pNNormal==1; }
    void addNormal(const IAVector3d&);
    void averageNormal();
    void print();
    void shrinkTo(double s);

    IAVector3d pInitialPosition;
    IAVector3d pPosition;
    IAVector3d pTex;
    IAVector3d pNormal;
    int pNNormal = 0;
};


typedef IAVertex *ISVertexPtr;
typedef std::vector<IAVertex*> IAVertexList;


#endif /* IA_VERTEX_H */


