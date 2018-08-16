//
//  IAGeometryReader.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAGeometryReader.h"
#include "IAGeometryReaderBinaryStl.h"
#include "IAGeometryReaderTextStl.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>


/**
 * Create a file reader for the indicated file.
 * \return 0 if the format is not supported
 */
std::shared_ptr<IAGeometryReader> IAGeometryReader::findReaderFor(const char *filename)
{
    std::shared_ptr<IAGeometryReader> reader = nullptr;
    // TODO: first look at the filename extension and prefer that file format
    if (!reader)
        reader = IAGeometryReaderBinaryStl::findReaderFor(filename);
    if (!reader)
        reader = IAGeometryReaderTextStl::findReaderFor(filename);
    // TODO: set error to Unsupported File Format
    return reader;
}


/**
 * Create a reader for the indicated memory block.
 * \return 0 if the format is not supported
 */
std::shared_ptr<IAGeometryReader> IAGeometryReader::findReaderFor(const char *name, unsigned char *data, size_t size)
{
    std::shared_ptr<IAGeometryReader> reader = nullptr;
    // TODO: first look at the name extension and prefer that file format
    if (!reader)
        reader = IAGeometryReaderBinaryStl::findReaderFor(name, data, size);
    if (!reader)
        reader = IAGeometryReaderTextStl::findReaderFor(name, data, size);
    // TODO: set error to Unsupported File Format
    return reader;
}


IAGeometryReader::IAGeometryReader(const char *filename)
{
    int fd = ::open(filename, O_RDONLY, 0);
    assert(fd != -1);
    if (fd==-1) {
        // panic
    }
    struct stat st; fstat(fd, &st);
    size_t len = st.st_size;
    ::lseek(fd, 0, SEEK_SET);

    pData = (uint8_t*)mmap(nullptr, len, PROT_READ,MAP_PRIVATE|MAP_FILE, fd, 0);
//    pData = (uint8_t*)mmap(nullptr, 0, PROT_READ, MAP_FILE, fd, 0);
    if (pData==MAP_FAILED) {
        perror("mmap");
        // panic
    }
    pCurrData = pData;
    pSize = len;
    pMustUnmapOnDelete = true;

    ::close(fd);
}


IAGeometryReader::IAGeometryReader(uint8_t *data, size_t size)
:   pData( data ),
    pCurrData( data ),
    pSize( size ),
    pMustUnmapOnDelete( false )
{
}


IAGeometryReader::~IAGeometryReader()
{
    if (pMustUnmapOnDelete) {
        ::munmap((void*)pData, pSize);
    }
}


/**
 Get a LSB first 32-bit word from memory.
 */
uint32_t IAGeometryReader::getUInt32LSB() {
    uint32_t ret = 0;
    ret |= *pCurrData++;
    ret |= (*pCurrData++)<<8;
    ret |= (*pCurrData++)<<16;
    ret |= (*pCurrData++)<<24;
    return ret;
}


/**
 Get a LSB first 16-bit word from memory.
 */
uint16_t IAGeometryReader::getUInt16LSB() {
    uint16_t ret = 0;
    ret |= *pCurrData++;
    ret |= (*pCurrData++)<<8;
    return ret;
}


float IAGeometryReader::getFloatLSB()
{
    float ret = *(const float*)pCurrData;
    pCurrData += 4;
    return ret;
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

