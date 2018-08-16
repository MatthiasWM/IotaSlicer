//
//  IAGeometryReader.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAGeometryReader.h"

#include <stdio.h>


/**
 * Create a file reader for the indicated file.
 * \return 0 if the format is not supported
 */
std::shared_ptr<IAGeometryReader> IAGeometryReader::findReaderFor(const char *filename)
{
    return nullptr;
}


/**
 * Create a reader for the indicated memory block.
 * \return 0 if the format is not supported
 */
std::shared_ptr<IAGeometryReader> IAGeometryReader::findReaderFor(const char *name, unsigned char *data, size_t size)
{
    return nullptr;
}


#include <tuple>

std::tuple<int, int> readByte() {
    return { 3, 1 };
}

void test()
{
    int x, err;
    std::tie(x, err) = readByte();
}

