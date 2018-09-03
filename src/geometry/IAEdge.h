//
//  IAEdge.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_EDGE_H
#define IA_EDGE_H


#include <vector>
#include <assert.h>


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
    friend IAMesh;
public:
    // new stuff
    IAHalfEdge(IATriangle *t, IAVertex *v);

    IAHalfEdge *twin() { return pTwin; }
    IAHalfEdge *prev() { return pPrev; }
    IAHalfEdge *next() { return pNext; }

    IATriangle *triangle() { return pTriangle; }
    IAVertex *vertex() { return pVertex; }

    IAHalfEdge *findNextSingleEdgeInFan();
    IAHalfEdge *findPrevSingleEdgeInFan();

    // old stuff
    IAVertex *findZGlobal(double);
    IAVertex *vertex(int i, IATriangle *f);
    IATriangle *otherTriangle(IATriangle *);
    IAVertex *otherVertex(IAVertex*);
    int indexIn(IATriangle *);
    int nTriangle();

    IATriangle *triangle(int i) {
        if (i==0) return pTriangle;
        if (i==1) {
            assert(pTwin);
            return twin()->triangle();
        }
        assert(0);
        return nullptr;
    }
    IAVertex *vertex(int i) {
        if (i==0) return vertex();
        if (i==1) {
            assert(pNext);
            return next()->vertex();
        }
        assert(0);
        return nullptr;
    }

protected:
    // new stuff
    void setTwin(IAHalfEdge *he) { pTwin = he; }
    void setPrev(IAHalfEdge *he) { pPrev = he; }
    void setNext(IAHalfEdge *he) { pNext = he; }

private:
    IAHalfEdge *pTwin = nullptr;
    IAHalfEdge *pPrev = nullptr;
    IAHalfEdge *pNext = nullptr;

    // old stuff
    IATriangle *pTriangle = nullptr;
    IAVertex *pVertex = nullptr;
};

typedef std::vector<IAHalfEdge*> IAHalfEdgeList;

typedef IAHalfEdge *IAHalfEdgePtr;


#endif /* IA_EDGE_H */


