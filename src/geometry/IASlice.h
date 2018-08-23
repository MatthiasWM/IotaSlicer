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

    bool changeZ(double z);
    void generateFlange(IAMesh*);
    void addFlange(IAMesh*);
    void addFirstFlangeVertex(IATriangle *IATriangle);
    void addNextFlangeVertex(IATrianglePtr &IATriangle, ISVertexPtr &vCutA, int &edgeIndex);
    void drawFlange();
    void drawShell();
    void drawFramebuffer();
    void tesselateLidFromFlange();


private:
    /// edge list describing the outlines of a slice
    IAEdgeList pFlange;
    /// current Z layer of the entire slice
    double pCurrentZ = -1e9;

public: // TODO: should not be public!
    IAFramebuffer *pFramebuffer = nullptr;
    IAFramebuffer *pColorbuffer = nullptr;
};


#endif /* IA_SLICE_H */


