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
class IAFace;


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
    IATriangle *otherFace(IATriangle *);
    IAVertex *otherVertex(IAVertex*);
    int indexIn(IATriangle *);
    int nFaces();

    IATriangle *pFace[2] = { nullptr, nullptr };
    IAVertex *pVertex[2] = { nullptr, nullptr };
};

typedef std::vector<IAEdge*> IAEdgeList;

#endif /* IA_EDGE_H */


