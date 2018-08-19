//
//  IASlice.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_SLICE_H
#define IA_SLICE_H


#include "IAMesh.h"


class IATriangle;
class IAFramebuffer;


/**
 * A mesh that represents a slice through another mesh at a give Z coordinate.
 *
 * \todo The pFlange edge list and the mesh are sharing the same vertexList in
 * IAMesh. This is not a big deal, but not very clean either.
 *
 * IAMesh has its own edge list that does not interfere with pFlange.
 */
class IASlice : public IAMesh
{
public:
    IASlice();
    virtual ~IASlice() override;
    virtual void clear() override;

    void generateFlange(IAMesh*);
    void addFlange(IAMesh*);
    void addFirstFlangeVertex(IATriangle *IATriangle);
    void addNextFlangeVertex(IATrianglePtr &IATriangle, ISVertexPtr &vCutA, int &edgeIndex);
    void drawFlange();
    void drawFramebuffer();

    // FIXME: in the process of fixing
    void tesselateLidFromFlange();

    // TODO: fix all functions below to use global space!

    void save(double z, const char *filename);

public:
    /// current Z layer of the entire slice
    /// \todo don't let external sources update this. The slice needs to know
    ///       itself when and how to update. Also, mark the slice invalid if
    ///       any of the global geometries of colors change.
    double pCurrentZ = -1e9;

private:
    /// edge list describing the outlines of a slice
    IAEdgeList pFlange;
public: // TODO: should not be public!
    IAFramebuffer *pFramebuffer = nullptr;
};


#endif /* IA_SLICE_H */


