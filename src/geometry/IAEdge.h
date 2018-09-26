//
//  IAEdge.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_EDGE_H
#define IA_EDGE_H


#include "app/IAMacros.h"

#include <vector>
#include <assert.h>


class IAVertex;
class IATriangle;
class IAMesh;


/**
 * A very minimal edge class that simply holds two vertices.
 *
 * This is used by IASlice to store teh mesh outline.
 */
class IAEdge
{
public:
    IAEdge();

    /** The two vertices that describe this edge */
    IAVertex *pVertex[2] = { nullptr, nullptr };
};

typedef std::vector<IAEdge*> IAEdgeList;


/**
 * Half-edges are used with triangles and vertices to describe meshes.
 *
 * Tow half-edges, twins, make up one full edge.
 */
class IAHalfEdge
{
    friend IAMesh;

public:
    IAHalfEdge(IATriangle *t, IAVertex *v);

    /** Two twins make up a full edge.
     \return the other half-edge that makes up this edge. */
    IAHalfEdge *twin() { return pTwin; }

    /** A triangle is a doubly linked list of half-edges.
     \return the previous edge in this triangle.
     The end of the previous half-edge is the start of this half-edge. */
    IAHalfEdge *prev() { return pPrev; }

    /** A triangle is a doubly linked list of half-edges.
     \return the next edge in this triangle.
     The end of this half-edge is the start of the next half-edge. */
    IAHalfEdge *next() { return pNext; }

    /** Triangles own and manage edges.
     \return the triangle that owns this half-edge. */
    IATriangle *triangle() { return pTriangle; }

    /** The vertex is the start position in space.
     \return the vertex that is the start of this half-edge.
     \todo half-edges must manage texture coordinates.*/
    IAVertex *vertex() { return pVertex; }

    IAHalfEdge *findNextSingleEdgeInFan();
    IAHalfEdge *findPrevSingleEdgeInFan();

    IAVertex *findZGlobal(double);

protected:
    /** Set the other half-edge that makes up this edge.
     \param he make this the twin */
    void setTwin(IAHalfEdge *he) { pTwin = he; }

    /** Set the previous half-edge for this triangle.
     \param he make this half-edge the previous half edge. */
    void setPrev(IAHalfEdge *he) { pPrev = he; }

    /** Set the next half-edge for this triangle.
     \param he make this half-edge the next half edge. */
    void setNext(IAHalfEdge *he) { pNext = he; }

private:
    /** Other half-edge, connecting two triangles. */
    IAHalfEdge *pTwin = nullptr;

    /** Previous half-edge in the triangle. */
    IAHalfEdge *pPrev = nullptr;

    /** Next half-edge in the triangle. */
    IAHalfEdge *pNext = nullptr;

    /** The owner of this half-edge. */
    IATriangle *pTriangle = nullptr;

    /** The starting point of this half-edge. */
    IAVertex *pVertex = nullptr;
};


typedef std::vector<IAHalfEdge*> IAHalfEdgeList;


typedef IAHalfEdge *IAHalfEdgePtr;


#endif /* IA_EDGE_H */


