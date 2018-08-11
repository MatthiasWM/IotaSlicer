//
//  IASlice.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_SLICE_H
#define IA_SLICE_H


#include "IAMesh.h"


class IATriangle;


class IASlice : public IAMesh
{
public:
    IASlice();
    virtual ~IASlice();
    virtual void clear();
    void generateFrom(IAMeshList&, double z);
    void drawLidEdge();
    void tesselate();
    void addZSlice(const IAMesh&, double);
    void addFirstLidVertex(IATriangle *IATriangle, double zMin);
    void addNextLidVertex(IATrianglePtr &IATriangle, ISVertexPtr &vCutA, int &edgeIndex, double zMin);

    IAEdgeList pLid;
};


#endif /* IA_SLICE_H */


