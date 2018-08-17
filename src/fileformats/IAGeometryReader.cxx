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
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#ifdef _WIN32
# include <io.h>
  static int open(const char *n, int f, int m) { return _open(n, f, m); }
  static void assert(bool v) { if (v==false) __debugbreak(); }
#else
# include <unistd.h>
# include <sys/mman.h>
#endif


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


/**
 * Create a universal reader by mapping the file to memory.
 */
IAGeometryReader::IAGeometryReader(const char *filename)
:   pName(strdup(filename))
{
    int fd = ::open(filename, O_RDONLY, 0);
    assert(fd != -1);
    if (fd==-1) {
        // panic
    }
    struct stat st; fstat(fd, &st);
    size_t len = st.st_size;
    ::lseek(fd, 0, SEEK_SET);

#ifdef _WIN32
	// TODO: use file mapping!
	pData = (uint8_t*)malloc(len);
	read(fd, pData, len);
#else
    pData = (uint8_t*)mmap(nullptr, len, PROT_READ,MAP_PRIVATE|MAP_FILE, fd, 0);
//    pData = (uint8_t*)mmap(nullptr, 0, PROT_READ, MAP_FILE, fd, 0);
    if (pData==MAP_FAILED) {
        perror("mmap");
        // panic
    }
#endif

    pCurrData = pData;
    pSize = len;
    pMustUnmapOnDelete = true;

    ::close(fd);
}


/**
 * Create a reader.
 */
IAGeometryReader::IAGeometryReader(const char *name, uint8_t *data, size_t size)
:   pData( data ),
    pCurrData( data ),
    pSize( size ),
    pMustUnmapOnDelete( false ),
    pName(strdup(name))
{
}


/**
 * Release any allocated resources.
 */
IAGeometryReader::~IAGeometryReader()
{
    if (pMustUnmapOnDelete) {
#ifdef _WIN32
		// FIXME: we should use file mapping instead
		free(pData);
#else
        ::munmap((void*)pData, pSize);
#endif
    }
    if (pName)
        free(pName);
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


/**
 * Get a 32bit float from memory
 */
float IAGeometryReader::getFloatLSB()
{
    float ret = *(const float*)pCurrData;
    pCurrData += 4;
    return ret;
}


/**
 * Skip the next n bytes when reading.
 */
void IAGeometryReader::skip(size_t n)
{
    pCurrData += n;
}


/**
 * Find the next keyword in a text file.
 */
bool IAGeometryReader::getWord()
{
    // skip whitespace
    // FIXME: test for end of buffer!
    for (;;) {
        uint8_t c = *pCurrData;
        if (c!=' ' && c!='\t' && c!='\r' && c!='\n')
            break;
        pCurrData++;
        if (pCurrData-pData > pSize)
            return false;
    }
    pCurrWord = pCurrData;
    char c = (char)*pCurrData;
    if (isalpha(c) || c=='_') {
        // find the end of a standard 'C' style keyword
        pCurrData++;
        for (;;) {
            char c = (char)*pCurrData;
            if (!isalnum(c) && c!='_')
                break;
            pCurrData++;
        }
        return true;
    }
    if (c=='"') {
        // find the end of a quoted string
        pCurrData++;
        for (;;) {
            char c = (char)*pCurrData;
            if (c=='\\')
                pCurrData++;
            else if (c=='"')
                break;
            pCurrData++;
        }
        return true;
    }
    if (c=='+' || c=='-' || c=='.' || isdigit(c)) {
        // find the end of a number
        pCurrData++;
        for (;;) {
            char c = (char)*pCurrData;
            if (!(isdigit(c) || c=='-' || c=='+' || c=='E' || c=='e' || c=='.'))
                break;
            pCurrData++;
        }
        return true;
    }
    pCurrData++;
    return true;
}


/**
 * Find the next keyword in a text file.
 */
double IAGeometryReader::getDouble()
{
    getWord();
    double ret = atof((char *)pCurrWord);
    return ret;
}


/**
 * Get the rest of this line
 */
bool IAGeometryReader::getLine()
{
    pCurrWord = pCurrData;
    for (;;) {
        uint8_t c = *pCurrData;
        if (c=='\r' || c=='\n')
            break;
        pCurrData++;
        if (pCurrData-pData > pSize)
            return false;
    }
    if ( (pCurrData-pData<=pSize) && pCurrData[0]=='\r' && pCurrData[1]=='\n')
        pCurrData++;
    pCurrData++;
    return true;
}


bool IAGeometryReader::wordIs(const char *key)
{
    size_t len = strlen(key);
    if (pCurrData-pCurrWord != len)
        return false;
    return (strncmp((char*)pCurrWord, key, len)==0);
}


void IAGeometryReader::printWord()
{
    size_t len = pCurrData-pCurrWord;
    printf("%.*s\n", (int)len, (char*)pCurrWord);
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

