//
//  IAGeometryReader.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_GEOMETRY_READER_H
#define IA_GEOMETRY_READER_H


#include "geometry/IAMesh.h"

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
    IAGeometryReader(const char *name, uint8_t *data, size_t size);
    virtual ~IAGeometryReader();

    /** Read some geometry form a file and generate a mesh.
     \return null, if we were not able to load a mesh. */
    virtual IAMesh *load() = 0;

protected:
    void skip(size_t n);
    uint32_t getUInt32LSB();
    uint16_t getUInt16LSB();
    float getFloatLSB();
    bool getWord();
    double getDouble();
    bool getLine();
    bool wordIs(const char *);
    void printWord();

    /** Filename for this reader.
     \return a ponter to the filename, don't free(). */
    const char *getName() const { return pName; }

private:
    bool pMustUnmapOnDelete = false;
    uint8_t *pData = nullptr;
    uint8_t *pCurrData = nullptr;
    uint8_t *pCurrWord = nullptr;
    size_t pSize = 0;
    char *pName = nullptr;

};


#endif /* IA_GEOMETRY_READER_H */
