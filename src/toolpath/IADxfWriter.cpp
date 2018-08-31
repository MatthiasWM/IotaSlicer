//
//  IADxfWriter.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IADxfWriter.h"

#include "Iota.h"
#include "data/binaryData.h"

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


bool IADxfWriter::open(const char *filename, int handseed)
{
    pFile = fopen(filename, "wb");
    if (!pFile) {
        // set error
        printf("Can't open file %s\n", filename);
        return false;
    }
#if 0
    fprintf(pFile, dxf_prolog, handseed+200);
#else
    fputs( // Header
    "999\r\n"
    "DXF created from Iota\r\n"
    "0\r\n"
    "SECTION\r\n"
    "2\r\n"
    "HEADER\r\n"
    "9\r\n"
    "$ACADVER\r\n"
    "1\r\n"
    "AC1006\r\n"
    "9\r\n"
    "$INSBASE\r\n"
    "10\r\n"
    "0.0\r\n"
    "20\r\n"
    "0.0\r\n"
    "30\r\n"
    "0.0\r\n"
    "9\r\n"
    "$EXTMIN\r\n"
    "10\r\n"
    "0.0\r\n"
    "20\r\n"
    "0.0\r\n"
    "9\r\n"
    "$EXTMAX\r\n"
    "10\r\n"
    "1000.0\r\n"
    "20\r\n"
    "1000.0\r\n"
    "0\r\n"
    "ENDSEC\r\n", pFile);
    fputs(
          "0\r\n"
          "SECTION\r\n"
          "2\r\n"
          "TABLES\r\n"
          "0\r\n"
          "TABLE\r\n"
          "2\r\n"
          "LTYPE\r\n"
          "70\r\n"
          "1\r\n"
          "0\r\n"
          "LTYPE\r\n"
          "2\r\n"
          "CONTINUOUS\r\n"
          "70\r\n"
          "64\r\n"
          "3\r\n"
          "Solid line\r\n"
          "72\r\n"
          "65\r\n"
          "73\r\n"
          "0\r\n"
          "40\r\n"
          "0.000000\r\n"
          "0\r\n"
          "ENDTAB\r\n"
          "0\r\n"
          "TABLE\r\n"
          "2\r\n"
          "LAYER\r\n"
          "70\r\n"
          "6\r\n"
          "0\r\n"
          "LAYER\r\n"
          "2\r\n"
          "1\r\n"
          "70\r\n"
          "64\r\n"
          "62\r\n"
          "7\r\n"
          "6\r\n"
          "CONTINUOUS\r\n"
          "0\r\n"
          "LAYER\r\n"
          "2\r\n"
          "2\r\n"
          "70\r\n"
          "64\r\n"
          "62\r\n"
          "7\r\n"
          "6\r\n"
          "CONTINUOUS\r\n"
          "0\r\n"
          "ENDTAB\r\n"
          "0\r\n"
          "TABLE\r\n"
          "2\r\n"
          "STYLE\r\n"
          "70\r\n"
          "0\r\n"
          "0\r\n"
          "ENDTAB\r\n"
          "0\r\n"
          "ENDSEC\r\n", pFile);
    fputs(
          "0\r\n"
          "SECTION\r\n"
          "2\r\n"
          "BLOCKS\r\n"
          "0\r\n"
          "ENDSEC\r\n", pFile);
    fputs(
          "0\r\n"
          "SECTION\r\n"
          "2\r\n"
          "ENTITIES\r\n", pFile);
#endif
    return true;
}


void IADxfWriter::cmdLine(IAVector3d &a, IAVector3d &b)
{
#if 0
    fprintf(pFile,
            "  0\r\nLINE\r\n"
            "  5\r\n%X\r\n"
            "330\r\n5D\r\n"
            "100\r\nAcDbEntity\r\n"
            "  8\r\n0\r\n"
            "100\r\nAcDbLine\r\n"
            " 10\r\n%f\r\n"
            " 20\r\n%f\r\n"
            " 30\r\n%f\r\n"
            " 11\r\n%f\r\n"
            " 21\r\n%f\r\n"
            " 31\r\n%f\r\n",
            pHandle++,
            a.x(), a.y(), 0.0,
            b.x(), b.y(), 0.0
            );
#else
    fprintf(pFile,
            "0\r\n"
            "LINE\r\n"
            "8\r\n"
            "2\r\n"
            "62\r\n"
            "4\r\n"
            "10\r\n"
            "%f\r\n"
            "20\r\n"
            "%f\r\n"
            "30\r\n"
            "%f\r\n"
            "11\r\n"
            "%f\r\n"
            "21\r\n"
            "%f\r\n"
            "31\r\n"
            "%f\r\n",
            a.x(), a.y(), 0.0,
            b.x(), b.y(), 0.0
            );
#endif
}


void IADxfWriter::close()
{
    if (pFile!=nullptr) {
#if 0
        fputs(dxf_epilog, pFile);
#else
        fputs(
              "0\r\n"
              "ENDSEC\r\n"
              "0\r\n"
              "EOF\r\n", pFile);

#endif
        fclose(pFile);
        pFile = nullptr;
    }
}




