//
//  IAFmtObjStl.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAGeometryReaderTextStl.h"

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
 *
 * \param filename read from this file.
 *
 * \return 0 if the format is not STL
 */
std::shared_ptr<IAGeometryReader> IAGeometryReaderTextStl::findReaderFor(const char *filename)
{
    int f = fl_open(filename, O_RDONLY);
    if (f==-1) {
        Iota.Error.set("STL Geometry reader", IAError::CantOpenFile_STR_BSD, filename);
        return nullptr;
    }
    
    uint8_t data[80];
    size_t n = ::read(f, data, 80);
    ::close(f);

    if (n<80) {
        Iota.Error.set("STL Geometry reader", IAError::UnknownFileType_STR, filename);
        return nullptr;
    }

    // skip leading whitespace
    char *src = (char*)data;
    // don;t skip more than 70 characters
    for (int i=0; i<70; i++) {
        char c = *src;
        if (c!=' ' && c!='\t' && c!='\r' && c!='\n')
            break;
        src++;
    }

    if (strncmp(src, "solid", 5)!=0) {
        Iota.Error.set("STL Geometry reader", IAError::UnknownFileType_STR, filename);
        return nullptr;
    }
    
    Iota.Error.clear();
    return std::make_shared<IAGeometryReaderTextStl>(filename);
}


/**
 * Create a reader for the indicated memory block.
 *
 * \param name similar to a filename, the extension of the name will help to
 *      determine the file type.
 * \param data verbatim copy of the file in memory
 * \param size number of bytes in that memory block
 *
 * \return 0 if the format is not STL
 *
 * \todo handle file reading errors
 */
std::shared_ptr<IAGeometryReader> IAGeometryReaderTextStl::findReaderFor(const char *name, uint8_t *data, size_t size)
{
    if (size<5)
        return nullptr;
    
    if (strncmp((char*)data, "solid", 5)!=0)
        return nullptr;
    
    return std::make_shared<IAGeometryReaderTextStl>(name, data, size);
}


/**
 * Create a file reader for reading from memory.
 *
 * \param name similar to a filename, the extension of the name will help to
 *      determine the file type.
 * \param data verbatim copy of the file in memory
 * \param size number of bytes in that memory block
 */
IAGeometryReaderTextStl::IAGeometryReaderTextStl(const char *name, uint8_t *data, size_t size)
:   IAGeometryReader(name, data, size)
{
}


/**
 * Create a file reader for reading from a file.
 *
 * \param filename read from this file
 */
IAGeometryReaderTextStl::IAGeometryReaderTextStl(const char *filename)
:   IAGeometryReader(filename)
{
}


/**
 * Release resources.
 */
IAGeometryReaderTextStl::~IAGeometryReaderTextStl()
{
}


/**
 * Interprete the geometry data and create a mesh list.
 *
 * \return nullptr, if the mesh could not be read or created.
 *
 * \todo gracefully handle outer loops with more than three vertices
 * \todo fix seams
 * \todo fix zero size holes
 * \todo fix degenrate triangles
 */
IAMesh *IAGeometryReaderTextStl::load()
{
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
    
    IAMesh *msh = new IAMesh();
    
    for (;;) {
        // the first word must be "solid"
        getWord();
        // we found STLs that contain mutiple solids!
        if (!wordIs("solid")) break;
        // the rest of the line is not important
        getLine();
        
        for (;;) {
            double x, y, z;
            IAVertex *p1, *p2, *p3;
            
            // check if this is the end of the facet list
            getWord();
            if ( wordIs("endsolid") ) {
                getLine();
                break;
            }
            
            // if this is not the end, it must be another facet
            if (!wordIs("facet")) goto fileFormatErr;
            getWord();
            
            // read the facet normal and ignore it
            if (!wordIs("normal")) goto fileFormatErr;
            getDouble();
            getDouble();
            getDouble();
            getWord();
            
            // is this the outer loop?
            if (!wordIs("outer")) goto fileFormatErr;
            getWord();
            if (!wordIs("loop")) goto fileFormatErr;
            getWord();
            
            // read the first vertex
            if (!wordIs("vertex")) goto fileFormatErr;
            x = getDouble();
            y = getDouble();
            z = getDouble();
            p1 = msh->findOrAddNewVertex(IAVector3d(x, y, z));
            p1->pTex.set(x*0.8+0.5, -z*0.8+0.5, 0.0);
            
            // read the second vertex
            getWord();
            if (!wordIs("vertex")) goto fileFormatErr;
            x = getDouble();
            y = getDouble();
            z = getDouble();
            p2 = msh->findOrAddNewVertex(IAVector3d(x, y, z));
            p2->pTex.set(x*0.8+0.5, -z*0.8+0.5, 0.0);
            
            // read the third vertex
            getWord();
            if (!wordIs("vertex")) goto fileFormatErr;
            x = getDouble();
            y = getDouble();
            z = getDouble();
            p3 = msh->findOrAddNewVertex(IAVector3d(x, y, z));
            p3->pTex.set(x*0.8+0.5, -z*0.8+0.5, 0.0);
            
            // this should be the end of the loop
            // Some files have additional vertices here, but that is agains the
            // standard. SO for now, we error out here. We could let it slip, but
            // to support this perfectly, we would need to tesselate the emtire
            // llop then.
            getWord();
            /** \todo word can actually be "vertex" to add more vertices to this polygon */
            if (!wordIs("endloop")) goto fileFormatErr;
            
            // this should be the end of the facet
            getWord();
            if (!wordIs("endfacet")) goto fileFormatErr;
            
            // add the triangle that was generated by those vertice
            msh->addNewTriangle(p1, p2, p3);
        }
    }
    if (!msh->validate()) {
        msh->fixHoles();
        msh->validate();
        /** \todo warn the user that the mesh could not be fixed! */
    }
    msh->calculateNormals();
    
    return msh;
    
fileFormatErr:
    Iota.Error.set("Read Text based STL File", IAError::FileContentCorrupt_STR, getName());
    delete msh;
    return nullptr;
}


