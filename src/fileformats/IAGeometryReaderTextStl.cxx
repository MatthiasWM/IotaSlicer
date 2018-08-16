//
//  IAFmtObjStl.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAGeometryReaderTextStl.h"


/**
 * Create a file reader for the indicated file.
 * \return 0 if the format is not STL
 */
std::shared_ptr<IAGeometryReader> IAGeometryReaderTextStl::findReaderFor(const char *filename)
{
    return nullptr;
}


/**
 * Create a reader for the indicated memory block.
 * \return 0 if the format is not STL
 */
std::shared_ptr<IAGeometryReader> IAGeometryReaderTextStl::findReaderFor(const char *name, uint8_t *data, size_t size)
{
    return nullptr;
}


