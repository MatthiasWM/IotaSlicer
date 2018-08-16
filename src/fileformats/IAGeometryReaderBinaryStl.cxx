//
//  IAFmtObjStl.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAGeometryReaderBinaryStl.h"

#include "Iota.h"
#include "../geometry/IAMesh.h"

#include <stdio.h>

/**
 Get a LSB first 16-bit word from a file.
 */
int getShort(FILE *f) {
    int ret = 0;
    ret |= fgetc(f);
    ret |= fgetc(f)<<8;
    return ret;
}


/**
 Get a LSB first 16-bit word from memory.
 */
int getShort(const unsigned char *&d) {
    int ret = 0;
    ret |= *d++;
    ret |= (*d++)<<8;
    return ret;
}


/**
 Get a LSB first 32-bit word from a file.
 */
int getInt(FILE *f) {
    int ret = 0;
    ret |= fgetc(f);
    ret |= fgetc(f)<<8;
    ret |= fgetc(f)<<16;
    ret |= fgetc(f)<<24;
    return ret;
}


/**
 Get a LSB first 32-bit word from memory.
 */
int getInt(const unsigned char *&d) {
    int ret = 0;
    ret |= *d++;
    ret |= (*d++)<<8;
    ret |= (*d++)<<16;
    ret |= (*d++)<<24;
    return ret;
}


/**
 Get a 32-bit float from a file.
 */
float getFloat(FILE *f) {
    float ret;
    fread(&ret, 4, 1, f);
    return ret;
}


/**
 Get a 32-bit float from memory.
 */
float getFloat(const unsigned char *&d) {
    float ret = *(const float*)d;
    d+=4;
    return ret;
}


/**
 Add a point to a mesh, avoiding duplicates.
 \todo: this should be a function of the mesh or its vertex list
 \todo: this must be accelerated by sorting vertices or better, using a map
 \todo: there should probably be a minimal tollerance when comparinf doubles!
 \return the index of the point in the mesh
 */
int addPoint(IAMesh *IAMesh, float x, float y, float z)
{
    int i, n = (int)IAMesh->vertexList.size();
    for (i = 0; i < n; ++i) {
        IAVertex *v = IAMesh->vertexList[i];
        if (   v->pPosition.x()==x
            && v->pPosition.y()==y
            && v->pPosition.z()==z)
        {
            return i;
        }
    }
    IAVertex *v = new IAVertex();
    v->pPosition.set(x, y, z);
    IAMesh->vertexList.push_back(v);
    return n;
}


/**
 Load a binary STL file fram a chunk of memory.

 STL triangles ar CCW, normals are pointing outward
 */
void loadStl(const unsigned char *d) {

    delete Iota.gMeshList;
    Iota.gMeshList = new IAMeshList;

    d+=0x50;
    IAMesh *msh = new IAMesh();
    Iota.gMeshList->push_back(msh);

    int nFaces = getInt(d);
    for (int i=0; i<nFaces; i++) {
        float x, y, z;
        int p1, p2, p3;
        // face normal
        getFloat(d);
        getFloat(d);
        getFloat(d);
        // point 1
        x = getFloat(d);
        y = getFloat(d);
        z = getFloat(d);
        p1 = addPoint(msh, x, y, z);
        msh->vertexList[p1]->pTex.set(x*0.8+0.5, -z*0.8+0.5, 0.0);
        // point 2
        x = getFloat(d);
        y = getFloat(d);
        z = getFloat(d);
        p2 = addPoint(msh, x, y, z);
        msh->vertexList[p2]->pTex.set(x*0.8+0.5, -z*0.8+0.5, 0.0);
        // point 3
        x = getFloat(d);
        y = getFloat(d);
        z = getFloat(d);
        p3 = addPoint(msh, x, y, z);
        msh->vertexList[p3]->pTex.set(x*0.8+0.5, -z*0.8+0.5, 0.0);
        // add face
        IATriangle *t = new IATriangle();
        t->pVertex[0] = msh->vertexList[p1];
        t->pVertex[1] = msh->vertexList[p2];
        t->pVertex[2] = msh->vertexList[p3];
        msh->addFace(t);
        // color

        msh->vertexList[p1]->pInitialPosition = msh->vertexList[p1]->pPosition;
        msh->vertexList[p2]->pInitialPosition = msh->vertexList[p2]->pPosition;
        msh->vertexList[p3]->pInitialPosition = msh->vertexList[p3]->pPosition;

        Iota.minX = min(Iota.minX, msh->vertexList[p1]->pPosition.x());
        Iota.maxX = max(Iota.maxX, msh->vertexList[p1]->pPosition.x());
        Iota.minY = min(Iota.minY, msh->vertexList[p1]->pPosition.y());
        Iota.maxY = max(Iota.maxY, msh->vertexList[p1]->pPosition.y());
        Iota.minZ = min(Iota.minZ, msh->vertexList[p1]->pPosition.z());
        Iota.maxZ = max(Iota.maxZ, msh->vertexList[p1]->pPosition.z());

        getShort(d);
    }

    msh->validate();
    // TODO: fix seams
    // TODO: fix zero size holes
    // TODO: fix degenrate triangles
    msh->fixHoles();
    msh->validate();

    msh->clearNormals();
    msh->calculateNormals();
}


/**
 Load a binary STL file fram a chunk of a file.
 */
void loadStl(const char *filename) {
    int i;

    delete Iota.gMeshList;
    Iota.gMeshList = new IAMeshList;

    FILE *f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "ERROR openening file!\n");
        return;
    }
    fseek(f, 0x50, SEEK_SET);
    IAMesh *msh = new IAMesh();
    Iota.gMeshList->push_back(msh);

    int nFaces = getInt(f);
    for (i=0; i<nFaces; i++) {
        float x, y, z;
        int p1, p2, p3;
        // face normal
        getFloat(f);
        getFloat(f);
        getFloat(f);
        // point 1
        x = getFloat(f);
        y = getFloat(f);
        z = getFloat(f);
        p1 = addPoint(msh, x, y, z);
        // point 2
        x = getFloat(f);
        y = getFloat(f);
        z = getFloat(f);
        p2 = addPoint(msh, x, y, z);
        // point 3
        x = getFloat(f);
        y = getFloat(f);
        z = getFloat(f);
        p3 = addPoint(msh, x, y, z);
        // add face
        IATriangle *t = new IATriangle();
        t->pVertex[0] = msh->vertexList[p1];
        t->pVertex[1] = msh->vertexList[p2];
        t->pVertex[2] = msh->vertexList[p3];
        msh->addFace(t);
        // color
        getShort(f);
    }

    msh->validate();
    // TODO: fix seams
    // TODO: fix zero size holes
    // TODO: fix degenrate triangles
    msh->fixHoles();
    msh->validate();

    msh->clearNormals();
    msh->calculateNormals();

    fclose(f);
}

