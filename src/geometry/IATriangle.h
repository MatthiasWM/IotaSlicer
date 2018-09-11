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

 Every geometry in Iota is reduced to triangles. There are no quads, polygons,
 or other higher level geometries.

 Triangles are defined by three half-edges, but they do not manage them.
 */
class IATriangle
{
public:
    IATriangle(IAMesh *m);
    bool crossesZGlobal(double z);

    /** Return one of tree vertices.
     \param i index of vertex
     \return pointer to the vertex */
    IAVertex *vertex(int i) const { return pEdge[i]->vertex(); }

    /** Return one of tree half-edges.
     \param i index of half-edge
     \return pointer to the half-edge */
    IAHalfEdge *edge(int i) const { return pEdge[i]; }

    /** Set all edges that make up this triangle.
     \param e0, e1, e2 half-edges */
    void setEdges(IAHalfEdge *e0, IAHalfEdge *e1, IAHalfEdge *e2) {
        pEdge[0] = e0; pEdge[1] = e1; pEdge[2] = e2; }

    /** Triangle face normal. */
    IAVector3d pNormal;

    /** Universal user flag, used to help find the slice circumference. */
    bool pUsed = false;

    /** Universal user flag, used to fix holes. */
    bool pPatched = false;

private:
    /** These half-edges define the triangle. */
    IAHalfEdge *pEdge[3] = { nullptr, nullptr, nullptr };

    /** This mesh manages this triangle. */
    IAMesh *pMesh = nullptr;
};

typedef IATriangle *IATrianglePtr;
typedef std::vector<IATriangle*> IATriangleList;


#endif /* IA_TRIANGLE_H */


