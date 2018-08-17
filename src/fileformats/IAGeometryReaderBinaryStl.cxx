//
//  IAFmtObjStl.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAGeometryReaderBinaryStl.h"

#include "Iota.h"
#include "../geometry/IAMesh.h"

#include <fcntl.h>
#ifdef _WIN32
# include <io.h>
  static int open(const char *n, int f, int m) { return _open(n, f, m); }
  static void assert(bool v) { if (v==false) __debugbreak(); }
#else
# include <unistd.h>
#endif


/**
 * Create a file reader for the indicated file.
 * \return 0 if the format is not STL
 */
std::shared_ptr<IAGeometryReader> IAGeometryReaderBinaryStl::findReaderFor(const char *filename)
{
    int f = ::open(filename, O_RDONLY);
    if (f==-1)  // TODO: set error
        return nullptr;

    // STL binary headers are not defined, except that they have an 80 character
    // ASCII header that must not start with "solid".
    uint8_t data[80];
    size_t n = ::read(f, data, 80);
    ::close(f);

    if (n<80)  // TODO: set error
        return nullptr;

    int i;
    for (i=0; i<80; i++) {
        if (data[i]>126) break;
    }
    if (i<80)  // TODO: set error
        return nullptr;

    if (strncmp((char*)data, "solid", 5)==0)   // TODO: set error
        return nullptr;

    return std::make_shared<IAGeometryReaderBinaryStl>(filename);
}


/**
 * Create a reader for the indicated memory block.
 * \return 0 if the format is not STL
 */
std::shared_ptr<IAGeometryReader> IAGeometryReaderBinaryStl::findReaderFor(const char *name, uint8_t *data, size_t size)
{
    if (size<80)
        return nullptr;

    int i;
    for (i=0; i<80; i++) {
        if (data[i]>126) break;
    }
    if (i<80)  // TODO: set error
        return nullptr;

    if (strncmp((char*)data, "solid", 5)==0)   // TODO: set error
        return nullptr;
    
    return std::make_shared<IAGeometryReaderBinaryStl>(data, size);
}



IAGeometryReaderBinaryStl::IAGeometryReaderBinaryStl(uint8_t *data, size_t size)
:   IAGeometryReader(data, size)
{
}


IAGeometryReaderBinaryStl::IAGeometryReaderBinaryStl(const char *filename)
:   IAGeometryReader(filename)
{
}


IAGeometryReaderBinaryStl::~IAGeometryReaderBinaryStl()
{
}


IAMeshList *IAGeometryReaderBinaryStl::load()
{
    IAMeshList *meshList = new IAMeshList;

    IAMesh *msh = new IAMesh();
    meshList->push_back(msh);

    skip(80);
    uint32_t nFaces = getUInt32LSB();
    for (int i=0; i<nFaces; i++) {
        float x, y, z;
        size_t p1, p2, p3;
        // face normal
        getFloatLSB();
        getFloatLSB();
        getFloatLSB();
        // point 1
        x = getFloatLSB();
        y = getFloatLSB();
        z = getFloatLSB();
        p1 = msh->addPoint(x, y, z);
        msh->vertexList[p1]->pTex.set(x*0.8+0.5, -z*0.8+0.5, 0.0);
        // point 2
        x = getFloatLSB();
        y = getFloatLSB();
        z = getFloatLSB();
        p2 = msh->addPoint(x, y, z);
        msh->vertexList[p2]->pTex.set(x*0.8+0.5, -z*0.8+0.5, 0.0);
        // point 3
        x = getFloatLSB();
        y = getFloatLSB();
        z = getFloatLSB();
        p3 = msh->addPoint(x, y, z);
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

        getUInt16LSB(); // color information, if there was a standard
    }

    msh->validate();
    // TODO: fix seams
    // TODO: fix zero size holes
    // TODO: fix degenrate triangles
    msh->fixHoles();
    msh->validate();

    msh->clearNormals();
    msh->calculateNormals();

    return meshList;
}


