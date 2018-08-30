//
//  IADxfWriter.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IADxfWriter.h"

#include "Iota.h"

#include <stdio.h>


/*
  0
LINE
  5
3C
330
5D
100
AcDbEntity
  8
0
100
AcDbLine
 10
 0.0
 20
 0.0
 30
 0.0
 11
 0.0
 21
 13.0
 31
 0.0
*/

IADxfWriter::IADxfWriter()
{
}


IADxfWriter::~IADxfWriter()
{
    if (pFile!=nullptr) close();
}


bool IADxfWriter::open(const char *filename)
{
    pFile = fopen(filename, "wb");
    if (!pFile) {
        // set error
        printf("Can't open file %s\n", filename);
        return false;
    }
    return true;
}


void IADxfWriter::cmdLine(IAVector3d &a, IAVector3d &b)
{
}


void IADxfWriter::close()
{
    if (pFile!=nullptr) {
        fclose(pFile);
        pFile = nullptr;
    }
}




