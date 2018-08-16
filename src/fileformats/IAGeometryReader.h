//
//  IAGeometryReader.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_GEOMETRY_READER_H
#define IA_GEOMETRY_READER_H


#include <stdio.h>
#include <memory>


/**
 * A class to lead any supported 3d geometry format.
 */
class IAGeometryReader
{
public:
    static std::shared_ptr<IAGeometryReader> findReaderFor(const char *filename);
    static std::shared_ptr<IAGeometryReader> findReaderFor(const char *name, unsigned char *data, size_t size);

public:
    IAGeometryReader() = default;
    virtual ~IAGeometryReader() = default;
};


#endif /* IA_GEOMETRY_READER_H */
