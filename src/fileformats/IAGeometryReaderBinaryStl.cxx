//
//  IAFmtObjStl.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAGeometryReaderBinaryStl.h"

#include "Iota.h"
#include "geometry/IAMesh.h"

#include <FL/fl_utf8.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef _WIN32
# include <io.h>
#else
# include <unistd.h>
#endif


/**
 * Create a file reader for the indicated file.
 *
 * Check if this is actually a binary STL. Thanks to awkward definition af this
 * format, the only somewhat reliable indicator, that this is actually a binary
 * STL, is to check the 32 bit word at offset 80, which indicates the number
 * af triangles, calculate the expected file size, and compare it to the
 * actual size of the file.
 *
 * \return 0 if the format is not STL
 */
#include <errno.h>
std::shared_ptr<IAGeometryReader> IAGeometryReaderBinaryStl::findReaderFor(const char *filename)
{
    struct stat fileStats;
    int ret = ::fl_stat(filename, &fileStats);
    if (ret!=0) {
        Iota.setError("STL Geometry reader", Error::CantOpenFile_STR_BSD, filename);
        return nullptr;
    }

    int f = fl_open(filename, O_RDONLY);
    if (f==-1) {
        Iota.setError("STL Geometry reader", Error::CantOpenFile_STR_BSD, filename);
        return nullptr;
    }

    // STL binary headers are not defined, except that they have an 80 character
    // ASCII header that must not start with "solid". This is not only a
    // terrible way to identify a file type, but also a lot of exporters simply
    // don't care, and feel like putting "solid" at the start of a *binary*
    // STL file is a great idea.

    // since the first 80 bytes are undefined and fille with whatver some
    // exporter happens to have in the buffer, and the rest of the files is a
    // bunch of vertices that can have pretty much any binary data in them,
    // we use the word that gives the number of triangle, calculate the size
    // of the file, and compare that to the actual size.

    // Mind you, this is still not good enough, because the STL Standard defines
    // the 16 bit "color" entry as a size field for the amount of bytes to
    // follow, which are supposed to describe color in some way. Luckily, no
    // exporter seems to use the field in that way (one exporter does store
    // an RGB value here though), so we should be fine.

    ::lseek(f, 80, SEEK_SET);
    uint8_t buf[4];
    size_t n = ::read(f, buf, 4);
    ::close(f);

    if (n<4) {
        Iota.setError("STL Geometry reader", Error::CantOpenFile_STR_BSD, filename);
        return nullptr;
    }

    size_t nTri = (size_t(buf[0]))
                + ((size_t(buf[1]))<<8)
                + ((size_t(buf[2]))<<16)
                + ((size_t(buf[3]))<<24);

    size_t expectedFileSize =
        nTri * (
               3*4  // point normal at three floats each
             + 9*4  // three vertices at three float coordinates each
             + 2)   // 16 bits for the triangle color
        + 80        // useless header
        +4;         // uint32_t containing the number of triangles in the file

    if ( expectedFileSize!=fileStats.st_size ) {
        Iota.setError("STL Geometry reader", Error::UnknownFileType_STR, filename);
        return nullptr;
    }

    Iota.clearError();
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


