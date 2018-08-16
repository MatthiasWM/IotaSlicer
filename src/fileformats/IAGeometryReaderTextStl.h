//
//  IAGeometryReaderTextStl.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_GEOMETRY_READER_TEXT_STL_H
#define IA_GEOMETRY_READER_TEXT_STL_H


#include "IAGeometryReader.h"


/**
 * This class reads an ASCII STL file and outputs a geometry.
 */
class IAGeometryReaderTextStl : public IAGeometryReader
{
    typedef IAGeometryReader super;

public:
    static std::shared_ptr<IAGeometryReader> findReaderFor(const char *filename);
    static std::shared_ptr<IAGeometryReader> findReaderFor(const char *name, uint8_t *data, size_t size);

    IAGeometryReaderTextStl() = default;
    ~IAGeometryReaderTextStl() = default;
};


#endif /* IA_GEOMETRY_READER_TEXT_STL_H */
