//
//  IAFmtObjStl.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAGeometryReaderBinaryStl.h"

#include "Iota.h"
#include "geometry/IAMesh.h"

#include <FL/fl_utf8.h>
#include <fcntl.h>
#ifdef _WIN32
# include <io.h>
#else
# include <unistd.h>
#endif


/**
 * Create a file reader for the indicated file.
 * \return 0 if the format is not STL
 */
#include <errno.h>
std::shared_ptr<IAGeometryReader> IAGeometryReaderBinaryStl::findReaderFor(const char *filename)
{
    int f = fl_open(filename, O_RDONLY);
    if (f==-1) {
        Iota.setError("STL Geometry reader", Error::CantOpenFile_STR_BSD, filename);
        return nullptr;
    }

    // STL binary headers are not defined, except that they have an 80 character
    // ASCII header that must not start with "solid".
    uint8_t data[80];
    size_t n = ::read(f, data, 80);
    ::close(f);

    if (n<80)  // TODO: set error
        return nullptr;

    if (strncmp((char*)data, "COLOR=", 6)==0) // there is some program that outputs this for binary SLT. Sigh!
        return std::make_shared<IAGeometryReaderBinaryStl>(filename);

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
    
    return std::make_shared<IAGeometryReaderBinaryStl>(name, data, size);
}



/**
 * Create a file reader for reading from memory.
 */
IAGeometryReaderBinaryStl::IAGeometryReaderBinaryStl(const char *name, uint8_t *data, size_t size)
:   IAGeometryReader(name, data, size)
{
}


/**
 * Create a file reader for reading from a file.
 */
IAGeometryReaderBinaryStl::IAGeometryReaderBinaryStl(const char *filename)
:   IAGeometryReader(filename)
{
}


/**
 * Release resources.
 */
IAGeometryReaderBinaryStl::~IAGeometryReaderBinaryStl()
{
}


/**
 * Interprete the geometry data and create a mesh list.
 */
IAMesh *IAGeometryReaderBinaryStl::load()
{
    IAMesh *msh = new IAMesh();

    skip(80);
    uint32_t nTriangle = getUInt32LSB();
    for (int i=0; i<nTriangle; i++) {
        float x, y, z;
        IAVertex *p1, *p2, *p3;
        // face normal
        getFloatLSB();
        getFloatLSB();
        getFloatLSB();
        // point 1
        x = getFloatLSB();
        y = getFloatLSB();
        z = getFloatLSB();
        p1 = msh->findOrAddNewVertex(IAVector3d(x, y, z));
        p1->pTex.set(x*0.8+0.5, -z*0.8+0.5, 0.0);
        // point 2
        x = getFloatLSB();
        y = getFloatLSB();
        z = getFloatLSB();
        p2 = msh->findOrAddNewVertex(IAVector3d(x, y, z));
        p2->pTex.set(x*0.8+0.5, -z*0.8+0.5, 0.0);
        // point 3
        x = getFloatLSB();
        y = getFloatLSB();
        z = getFloatLSB();
        p3 = msh->findOrAddNewVertex(IAVector3d(x, y, z));
        p3->pTex.set(x*0.8+0.5, -z*0.8+0.5, 0.0);
        // add face
        msh->addNewTriangle(p1, p2, p3);
        // color
        getUInt16LSB(); // color information, if there was a standard
    }

    // FIXME: fixing the mesh should be done after loading *any* mesh, not just this one
    msh->validate();
    // TODO: fix seams
    // TODO: fix zero size holes
    // TODO: fix degenrate triangles
    msh->fixHoles();
    msh->validate();

    msh->clearNormals();
    msh->calculateNormals();

    return msh;
}


