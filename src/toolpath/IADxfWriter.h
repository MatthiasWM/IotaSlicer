//
//  IADxfWriter.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_DXF_WRITER_H
#define IA_DXF_WRITER_H


#include "geometry/IAVector3d.h"

#include <vector>
#include <map>


/**
 * Helps the toolpath classes to write DXF files.
 */
class IADxfWriter
{
public:
    IADxfWriter();
    ~IADxfWriter();

    bool open(const char *filename);
    void close();

    void cmdLine(IAVector3d &a, IAVector3d &b);

private:
    FILE *pFile = nullptr;
    int pHandle = 0x3C;
};


#endif /* IA_DXF_WRITER_H */


