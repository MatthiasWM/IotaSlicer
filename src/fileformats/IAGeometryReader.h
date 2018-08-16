//
//  IAGeometryReader.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_GEOMETRY_READER_H
#define IA_GEOMETRY_READER_H


#include "../geometry/IAMesh.h"

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
    IAGeometryReader(const char *filename);
    IAGeometryReader(uint8_t *data, size_t size);
    virtual ~IAGeometryReader();
    virtual IAMeshList *load() = 0;

protected:
    void skip(size_t n) { pCurrData += n; }
    uint32_t getUInt32LSB();
    uint16_t getUInt16LSB();
    float getFloatLSB();

private:
    bool pMustUnmapOnDelete = false;
    uint8_t *pData = nullptr;
    uint8_t *pCurrData = nullptr;
    size_t pSize = 0;
};


#endif /* IA_GEOMETRY_READER_H */
