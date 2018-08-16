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
    IAGeometryReaderBinaryStl() = default;
    ~IAGeometryReaderBinaryStl() = default;
};



extern void loadStl(const unsigned char *d);
extern void loadStl(const char *filename);



#endif /* IA_GEOMETRY_READER_BINARY_STL_H */
