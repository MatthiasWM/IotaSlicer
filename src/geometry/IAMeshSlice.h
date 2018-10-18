//
//  IASlice.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_SLICE_H
#define IA_SLICE_H


#include "IAMesh.h"

class IAPrinter;
class IATriangle;
class IAFramebuffer;


/**
 * A mesh that represents a slice through another mesh at a give Z coordinate.
 *
 * \todo The pRim edge list and the mesh are sharing the same vertexList in
 * IAMesh. This is not a big deal, but not very clean either.
 *
 * IAMesh has its own edge list that does not interfere with pRim.
 *
 * \todo framebuffer member variables should not be public!
 */
class IAMeshSlice : public IAMesh
{
public:
    IAMeshSlice(IAPrinter*);
    virtual ~IAMeshSlice() override;
    virtual void clear() override;
    bool setNewZ(double z);

    void generateRim(IAMesh*);
    void addRim(IAMesh*);
    void addFirstRimVertex(IATriangle *IATriangle);
    bool addNextRimVertex(IAHalfEdgePtr &edge);
    void drawRim();
    void tesselateAndDrawLid();
    void drawShell();
    void drawFramebuffer();
    void tesselateLidFromRim();

private:
    /// edge list describing the outlines of a slice
    IAEdgeList pRim;
    /// current Z layer of the entire slice
    double pCurrentZ = -1e9;
    /// link back to the printer that created the slice, so we can retreive the build volume
    //IAPrinter *pPrinter = nullptr;

public:
    IAFramebuffer *pFramebuffer = nullptr;
    IAFramebuffer *pColorbuffer = nullptr;
};


#endif /* IA_SLICE_H */


