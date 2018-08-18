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

    void generateFlange(IAMesh*);
    void addFlange(IAMesh*);
    void addFirstLidVertex(IATriangle *IATriangle);
    void addNextLidVertex(IATrianglePtr &IATriangle, ISVertexPtr &vCutA, int &edgeIndex);

    // FIXME: in the process of fixing
    void drawFlange();

    // TODO: fix all functions below to use global space!
    void generateLid(IAMesh*, double z);
    void tesselate();

    void save(double z, const char *filename);

    /// edge list describin the outlines of a slice
    IAEdgeList pFlange;
    double pCurrentZ = -1e9;
};


#endif /* IA_SLICE_H */


