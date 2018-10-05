//
//  IAMesh.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_MESH_H
#define IA_MESH_H


#include "IAVertex.h"
#include "IATriangle.h"
#include "IAEdge.h"

#include <vector>
#include <map>
#include <float.h>


class IAPrinter;

typedef std::multimap<double, IAVertex*> IAVertexMap;
typedef std::multimap<double, IAHalfEdge*> IAHalfEdgeMap;


/**
 A mesh represents a single geometric object, made out of vertices and triangles.

 Every geometry need to be converted into such a mesh. Geometries that contain
 multiple separate objects should be converted into multiple meshes.

 The vertex list should not have two or more points at the same coordinate.
 Every vertex can be connected to any number of edges and triangles.

 The Mesh manages the vertex list, the triangle list, and the edge list.
 */
class IAMesh
{
public:
    IAMesh();
    virtual ~IAMesh() { clear(); }
    virtual void clear();
    bool validate();
    void drawGouraud();
    void drawFlat(bool textured=false, float r=0.6f, float g=0.6, float b=0.6, float a=1.0);
    void drawAngledFaces(double a);
//    void drawShrunk(unsigned int, double);
    void drawEdges();
    void drawSliced(double z);
    void drawSlicedGhost(double z);

    /** Calculate all face normals and all point normals. */
    void calculateNormals() { calculateTriangleNormals(); calculateVertexNormals(); }
    
    void fixHoles();
    void fixHole(IAHalfEdge*);
//    void shrinkBy(double s);
    void projectTexture(double w, double h, int type);

    IATriangle *addNewTriangle(IAVertex *v0, IAVertex *v1, IAVertex *v2);

    IAHalfEdge *findEdge(IAVertex*, IAVertex*);
    IAHalfEdge *findSingleEdge(IAVertex*, IAVertex*);
    IAHalfEdge *addHalfEdge(IAHalfEdge*);
    IAVertex *findOrAddNewVertex(IAVector3d const&);

    void updateBoundingBox(IAVector3d const&);
    void centerOnPrintbed(IAPrinter *printer);

    IAVector3d position() const;
    void position(const IAVector3d &p);

    void updateGlobalSpace();

    /** List of vertices for fast access through indexing. */
    IAVertexList vertexList;

    /** Map of vertices for fast access through the vertex position. */
    IAVertexMap vertexMap;

    /** List of all half-edges in the mesh. */
    IAHalfEdgeList edgeList;

    /** Mpa of all half-edges for finding twins and duplicates quickly. */
    IAHalfEdgeMap edgeMap;

    /** List of all triangles in this mesh */
    IATriangleList triangleList;

    /** Smallest coordinte of all vertices in the mesh in mesh space. */
    IAVector3d pMin = { FLT_MAX, FLT_MAX, FLT_MAX};

    /** Largest coordinte of all vertices in the mesh in mesh space. */
    IAVector3d pMax = { FLT_MIN, FLT_MIN, FLT_MIN };

private:
    void clearVertexNormals();
    void calculateTriangleNormals();
    void calculateVertexNormals();

    /** This is true whenever pGlobalPosition and pGlobalNormal need to be recalculated */
    bool pGlobalPositionNeedsUpdate = true;

    /// Position of this object in scene space
    /// \todo we also need rotation and scale
    IAVector3d pMeshPosition;

};


#endif /* IA_MESH_H */


