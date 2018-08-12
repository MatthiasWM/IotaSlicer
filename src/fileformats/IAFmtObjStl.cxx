//
//  IAFmtObjStl.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAFmtObjStl.h"

#include "main.h"
#include "../geometry/IAMesh.h"

#include <stdio.h>

int getShort(FILE *f) {
    int ret = 0;
    ret |= fgetc(f);
    ret |= fgetc(f)<<8;
    return ret;
}

int getShort(const unsigned char *&d) {
    int ret = 0;
    ret |= *d++;
    ret |= (*d++)<<8;
    return ret;
}

int getInt(FILE *f) {
    int ret = 0;
    ret |= fgetc(f);
    ret |= fgetc(f)<<8;
    ret |= fgetc(f)<<16;
    ret |= fgetc(f)<<24;
    return ret;
}

int getInt(const unsigned char *&d) {
    int ret = 0;
    ret |= *d++;
    ret |= (*d++)<<8;
    ret |= (*d++)<<16;
    ret |= (*d++)<<24;
    return ret;
}

float getFloat(FILE *f) {
    float ret;
    fread(&ret, 4, 1, f);
    return ret;
}

float getFloat(const unsigned char *&d) {
    float ret = *(const float*)d;
    d+=4;
    return ret;
}

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


// STL triangles ar CCW, normals are pointing outward
void loadStl(const unsigned char *d) {
    d+=0x50;
    IAMesh *msh = new IAMesh();
    gMeshList.push_back(msh);

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

        minX = min(minX, msh->vertexList[p1]->pPosition.x());
        maxX = max(maxX, msh->vertexList[p1]->pPosition.x());
        minY = min(minY, msh->vertexList[p1]->pPosition.y());
        maxY = max(maxY, msh->vertexList[p1]->pPosition.y());
        minZ = min(minZ, msh->vertexList[p1]->pPosition.z());
        maxZ = max(maxZ, msh->vertexList[p1]->pPosition.z());

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
 Load a single node from a binary stl file.
 */
void loadStl(const char *filename) {
    int i;

    FILE *f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "ERROR openening file!\n");
        return;
    }
    fseek(f, 0x50, SEEK_SET);
    IAMesh *msh = new IAMesh();
    gMeshList.push_back(msh);

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
#ifndef M_XYZ
    msh->fixHoles();
#endif
    msh->validate();

    msh->clearNormals();
    msh->calculateNormals();

    fclose(f);
}

