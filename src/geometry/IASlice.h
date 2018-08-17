//
//  IASlice.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_SLICE_H
#define IA_SLICE_H


#include "IAMesh.h"


class IATriangle;


/**
 A mesh that represents a slice through another mesh at a give Z coordinate.
 */
class IASlice : public IAMesh
{
public:
    IASlice();
    virtual ~IASlice() override;
    virtual void clear() override;
    void generateLidFrom(IAMesh*, double z);
    void generateOutlineFrom(IAMesh*, double z);
    void drawLidEdge();
    void tesselate();
    void addZSlice(IAMesh*, double);
    void addFirstLidVertex(IATriangle *IATriangle, double zMin);
    void addNextLidVertex(IATrianglePtr &IATriangle, ISVertexPtr &vCutA, int &edgeIndex, double zMin);

    void save(double z, const char *filename);

    IAEdgeList pLid;
    double pCurrentZ = -1e9;
};


#endif /* IA_SLICE_H */


