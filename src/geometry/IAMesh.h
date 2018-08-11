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


class IAMesh
{
public:
    IAMesh();
    virtual ~IAMesh() { clear(); }
    virtual void clear();
    bool validate();
    void drawGouraud();
    void drawFlat(unsigned int);
    void drawShrunk(unsigned int, double);
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
    void shrinkTo(double s);
    IAEdge *findEdge(IAVertex*, IAVertex*);
    IAEdge *addEdge(IAVertex*, IAVertex*, IATriangle*);

    IAVertexList vertexList;
    IAEdgeList edgeList;
    IATriangleList faceList;
};


typedef std::vector<IAMesh*> IAMeshVector;


class IAMeshList
{
public:
    IAMeshList() { }
    int size() { return (int)meshList.size(); }
    IAMesh *operator[](int ix) { return meshList[ix]; }
    void push_back(IAMesh *mesh) { meshList.push_back(mesh); }
    void drawFlat(unsigned int color);
    void shrinkTo(double s);
private:
    IAMeshVector meshList;
};


#endif /* IA_MESH_H */


