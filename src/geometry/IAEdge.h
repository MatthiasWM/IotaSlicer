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

class IAEdge
{
public:
    IAEdge();
    IAVertex *findZ(double);
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


