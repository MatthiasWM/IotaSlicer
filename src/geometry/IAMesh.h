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
#include <float.h>


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
    void drawFlat(float r=0.8f, float g=0.8, float b=0.8, float a=1.0);
//    void drawShrunk(unsigned int, double);
    void drawEdges();
    void addFace(IATriangle*);
    void clearFaceNormals();
    void clearVertexNormals();
    void clearNormals() { clearFaceNormals(); clearVertexNormals(); }
    void calculateFaceNormals();
    void calculateVertexNormals();
    void calculateNormals() { calculateFaceNormals(); calculateVertexNormals(); }
    void fixHoles();
    void fixHole(IAEdge*);
    void shrinkBy(double s);
    void projectTexture(double w, double h, int type);
    IAEdge *findEdge(IAVertex*, IAVertex*);
    IAEdge *addEdge(IAVertex*, IAVertex*, IATriangle*);
    size_t addPoint(double x, double y, double z);

    void updateBoundingBox(IAVector3d&);

    IAVertexList vertexList;
    IAEdgeList edgeList;
    IATriangleList faceList;
    IAVector3d pMin = { FLT_MAX, FLT_MAX, FLT_MAX};
    IAVector3d pMax = { FLT_MIN, FLT_MIN, FLT_MIN };
};


typedef std::vector<IAMesh*> IAMeshVector;
typedef IAMeshVector::iterator IAMeshVectorIt;


/**
 Manage a list of meshes.
 */
class IAMeshList
{
public:
    IAMeshList() { }
    ~IAMeshList();
    int size() { return (int)meshList.size(); }
    IAMesh *operator[](int ix) { return meshList[ix]; }
    void push_back(IAMesh *mesh) { meshList.push_back(mesh); }
    void drawFlat(bool textured=false, float r=0.6f, float g=0.6, float b=0.6, float a=1.0);
    void drawGouraud();
    void drawSliced(double z);
    void shrinkBy(double s);
    void projectTexture(double w, double h, int type);
    void updateBoundingBox();

    IAVector3d pMin = { FLT_MAX, FLT_MAX, FLT_MAX};
    IAVector3d pMax = { FLT_MIN, FLT_MIN, FLT_MIN };

private:
    IAMeshVector meshList;
};


#endif /* IA_MESH_H */


