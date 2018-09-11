//
//  IATriangle.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_TRIANGLE_H
#define IA_TRIANGLE_H


#include "IAVector3d.h"
#include "IAEdge.h"

#include <vector>


class IAMesh;
class IAVertex;
class IAHalfEdge;


/**
 Represent a triangle inside a mesh.

 Every geometry in Ioat is reduced to triangles. There are no quads, polygons,
 or even higher level geometries.

 Triangles share points and share edges, but they do not manage them.
 */
class IATriangle
{
public:
    IATriangle(IAMesh *m);
    bool crossesZGlobal(double z);
    int pointsBelowZGlobal(double z);

    IAVertex *vertex(int i) const { return pEdge[i]->vertex(); }
    IAHalfEdge *edge(int i) const { return pEdge[i]; }
    void setEdges(IAHalfEdge *e0, IAHalfEdge *e1, IAHalfEdge *e2) {
        pEdge[0] = e0; pEdge[1] = e1; pEdge[2] = e2; }

    IAVector3d pNormal;
    int pNNormal = 0;
    bool pUsed = false;
    bool pPatched = false;

private:
    IAHalfEdge *pEdge[3] = { nullptr, nullptr, nullptr };
    IAMesh *pMesh = nullptr;
};

typedef IATriangle *IATrianglePtr;
typedef std::vector<IATriangle*> IATriangleList;


#endif /* IA_TRIANGLE_H */


