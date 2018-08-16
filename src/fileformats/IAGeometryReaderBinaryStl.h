//
//  IAGeometryReaderBinaryStl.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_GEOMETRY_READER_BINARY_STL_H
#define IA_GEOMETRY_READER_BINARY_STL_H


#include "IAGeometryReader.h"


/**
 * This class reads an ASCII STL file and outputs a geometry.
 */
class IAGeometryReaderBinaryStl : public IAGeometryReader
{
    typedef IAGeometryReader super;
    
public:
    static std::shared_ptr<IAGeometryReader> findReaderFor(const char *filename);
    static std::shared_ptr<IAGeometryReader> findReaderFor(const char *name, uint8_t *data, size_t size);

    IAGeometryReaderBinaryStl(uint8_t *data, size_t size);
    IAGeometryReaderBinaryStl(const char *filename);
    virtual ~IAGeometryReaderBinaryStl() override;
    virtual IAMeshList *load() override;
};


#endif /* IA_GEOMETRY_READER_BINARY_STL_H */
