//
//  IAEdge.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_EDGE_H
#define IA_EDGE_H


#include <vector>


class IAVertex;
class IATriangle;
class IAMesh;


/**
 Represent the boundary where two triangles meet.

 In a perfectly watertight mesh, every triangle has three connected triangles,
 three edges, and every edge connects to this and one other triangle.

 This class helps walking the mesh. It does not manage any faces or vertices.
 */
class IAEdge
{
public:
    IAEdge();
    IAVertex *findZGlobal(double);
    IAVertex *vertex(int i, IATriangle *f);
    IATriangle *otherTriangle(IATriangle *);
    IAVertex *otherVertex(IAVertex*);
    int indexIn(IATriangle *);
    int nTriangle();

    IATriangle *pTriangle[2] = { nullptr, nullptr };
    IAVertex *pVertex[2] = { nullptr, nullptr };
};

typedef std::vector<IAEdge*> IAEdgeList;


class IAHalfEdge
{
public:
    // new stuff
    IAHalfEdge *twin() { return pTwin; }
    IAHalfEdge *prev() { return pPrev; }
    IAHalfEdge *next() { return pNext; }

    // old stuff
    IAHalfEdge();

    IAVertex *findZGlobal(double);
    IAVertex *vertex(int i, IATriangle *f);
    IATriangle *otherTriangle(IATriangle *);
    IAVertex *otherVertex(IAVertex*);
    int indexIn(IATriangle *);
    int nTriangle();

    IATriangle *triangle(int i) { return pTriangle[i]; }
    void setTriangle(int i, IATriangle *t) { pTriangle[i] = t; }
    IAVertex *vertex(int i) { return pVertex[i]; }
    void setVertex(int i, IAVertex *v) { pVertex[i] = v; }

protected:
    // new stuff
    void setTwin(IAHalfEdge *he) { pTwin = he; }
    void setPrev(IAHalfEdge *he) { pPrev = he; }
    void setNext(IAHalfEdge *he) { pNext = he; }

private:
    IAHalfEdge *pTwin = nullptr; // FIXME: continue here, implement these
    IAHalfEdge *pPrev = nullptr;
    IAHalfEdge *pNext = nullptr;

    // old stuff
    IATriangle *pTriangle[2] = { nullptr, nullptr };
    IAVertex *pVertex[2] = { nullptr, nullptr };
};

typedef std::vector<IAHalfEdge*> IAHalfEdgeList;



#endif /* IA_EDGE_H */


