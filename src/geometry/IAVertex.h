//
//  IAVertex.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_VERTEX_H
#define IA_VERTEX_H

#include "IAVector3d.h"

#include <vector>


/**
 A point in space that can be used by meshes, triangles, and edges.
 */
class IAVertex
{
public:
    IAVertex();
    IAVertex(const IAVertex*);
    bool validNormal() { return pNNormal==1; }
    void addNormal(const IAVector3d&);
    void averageNormal();
    void print();
    void shrinkBy(double s);
    void projectTexture(double w, double h, int type);

    IAVector3d pInitialPosition;
    IAVector3d pPosition;
    IAVector3d pTex;
    IAVector3d pNormal = { 1.0, 0.0, 0.0 };
    int pNNormal = 0;
};


typedef IAVertex *ISVertexPtr;
typedef std::vector<IAVertex*> IAVertexList;


#endif /* IA_VERTEX_H */


