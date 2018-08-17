//
//  IAFmtObjStl.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAGeometryReaderTextStl.h"

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
std::shared_ptr<IAGeometryReader> IAGeometryReaderTextStl::findReaderFor(const char *filename)
{
    int f = ::open(filename, O_RDONLY);
    if (f==-1)  // TODO: set error
        return nullptr;

    uint8_t data[5];
    size_t n = ::read(f, data, 5);
    ::close(f);

    if (n<5)  // TODO: set error
        return nullptr;

    if (strncmp((char*)data, "solid", 5)!=0)   // TODO: set error
        return nullptr;

    return std::make_shared<IAGeometryReaderTextStl>(filename);
}


/**
 * Create a reader for the indicated memory block.
 * \return 0 if the format is not STL
 */
std::shared_ptr<IAGeometryReader> IAGeometryReaderTextStl::findReaderFor(const char *name, uint8_t *data, size_t size)
{
    if (size<5)  // TODO: set error
        return nullptr;

    if (strncmp((char*)data, "solid", 5)!=0)   // TODO: set error
        return nullptr;

    return std::make_shared<IAGeometryReaderTextStl>(data, size);
}


IAGeometryReaderTextStl::IAGeometryReaderTextStl(uint8_t *data, size_t size)
:   IAGeometryReader(data, size)
{
}


IAGeometryReaderTextStl::IAGeometryReaderTextStl(const char *filename)
:   IAGeometryReader(filename)
{
}


IAGeometryReaderTextStl::~IAGeometryReaderTextStl()
{
}


IAMeshList *IAGeometryReaderTextStl::load()
{
    IAMeshList *meshList = new IAMeshList;

    IAMesh *msh = new IAMesh();
    meshList->push_back(msh);

    /*
        solid name
        facet normal ni nj nk
            outer loop
                vertex v1x v1y v1z
                vertex v2x v2y v2z
                vertex v3x v3y v3z
            endloop
        endfacet
     */

    getWord();
    if (!wordIs("solid"))
        assert(1);
    getLine();

    for (;;) {
        double x, y, z;
        size_t p1, p2, p3;
        
        getWord();
        if ( wordIs("endsolid") )
            break;
        assert( wordIs("facet") );
        getWord();
        assert( wordIs("normal") );
        getDouble();
        getDouble();
        getDouble();
        getWord();
        assert( wordIs("outer") );
        getWord();
        assert( wordIs("loop") );
        getWord();
        assert( wordIs("vertex") );
        x = getDouble();
        y = getDouble();
        z = getDouble();
        p1 = msh->addPoint(x, y, z);
        msh->vertexList[p1]->pTex.set(x*0.8+0.5, -z*0.8+0.5, 0.0);

        getWord();
        assert( wordIs("vertex") );
        x = getDouble();
        y = getDouble();
        z = getDouble();
        p2 = msh->addPoint(x, y, z);
        msh->vertexList[p2]->pTex.set(x*0.8+0.5, -z*0.8+0.5, 0.0);

        getWord();
        assert( wordIs("vertex") );
        x = getDouble();
        y = getDouble();
        z = getDouble();
        p3 = msh->addPoint(x, y, z);
        msh->vertexList[p3]->pTex.set(x*0.8+0.5, -z*0.8+0.5, 0.0);

        getWord();
        // FIXME: word can actually be "vertex" to add more vertices to this polygon
        assert( wordIs("endloop") );

        getWord();
        assert( wordIs("endfacet") );

        // add face
        IATriangle *t = new IATriangle();
        t->pVertex[0] = msh->vertexList[p1];
        t->pVertex[1] = msh->vertexList[p2];
        t->pVertex[2] = msh->vertexList[p3];
        msh->addFace(t);

        msh->vertexList[p1]->pInitialPosition = msh->vertexList[p1]->pPosition;
        msh->vertexList[p2]->pInitialPosition = msh->vertexList[p2]->pPosition;
        msh->vertexList[p3]->pInitialPosition = msh->vertexList[p3]->pPosition;

        Iota.minX = min(Iota.minX, msh->vertexList[p1]->pPosition.x());
        Iota.maxX = max(Iota.maxX, msh->vertexList[p1]->pPosition.x());
        Iota.minY = min(Iota.minY, msh->vertexList[p1]->pPosition.y());
        Iota.maxY = max(Iota.maxY, msh->vertexList[p1]->pPosition.y());
        Iota.minZ = min(Iota.minZ, msh->vertexList[p1]->pPosition.z());
        Iota.maxZ = max(Iota.maxZ, msh->vertexList[p1]->pPosition.z());
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


/**
 */

