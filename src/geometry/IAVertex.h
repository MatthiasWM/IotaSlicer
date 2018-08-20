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
    void projectTexture(double w, double h, int type);

    /// Point position in object space, as it comes from the 3d object file
    IAVector3d pLocalPosition;
    /// Point position in scene space, used by IASlice
    IAVector3d pGlobalPosition;
    /// Inner shell point position in scene space
    // IAVector3d pShrunkPosition;
    IAVector3d pTex;
    IAVector3d pNormal = { 1.0, 0.0, 0.0 };
    /// Point normal in object space
    // IAVector3d pLocalNormal;
    /// Point normal in scene space
    // IAVector3d pGlobalNormal;
    int pNNormal = 0;
};


typedef IAVertex *ISVertexPtr;
typedef std::vector<IAVertex*> IAVertexList;


#endif /* IA_VERTEX_H */


