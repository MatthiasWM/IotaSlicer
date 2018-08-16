//
//  IAGeometryReader.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_GEOMETRY_READER_H
#define IA_GEOMETRY_READER_H


#include <stdio.h>


/**
 * A class to lead any supported 3d geometry format.
 */
class IAGeometryReader
{
public:
    static IAGeometryReader *findReader(const char *filename);
    static IAGeometryReader *findReader(const char *name, unsigned char *data, size_t size);
};


#endif /* IA_GEOMETRY_READER_H */
