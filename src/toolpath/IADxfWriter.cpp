//
//  IADxfWriter.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IADxfWriter.h"

#include "Iota.h"
#include "data/binaryData.h"

#include <stdio.h>


/**
 * Create a DXF file writer.
 */
IADxfWriter::IADxfWriter()
{
}


/**
 * Release all resources and close file.
 */
IADxfWriter::~IADxfWriter()
{
    if (pFile!=nullptr) close();
}


/**
 * Create a minimal DXF file for writing.
 *
 * \param filename path and name for the file we want to create.
 *
 * \return true, if the file was created, false, if there was any kind of error.
 *
 * \todo add user friendly error handling.
 */
bool IADxfWriter::open(const char *filename)
{
    pFile = fopen(filename, "wb");
    if (!pFile) {
        // set error
        printf("Can't open file %s\n", filename);
        return false;
    }
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
    return true;
}


/**
 * Write the command to create a line into a DXF file.
 *
 * \param a, b start and end coordinte of the line.
 */
void IADxfWriter::cmdLine(IAVector3d &a, IAVector3d &b)
{
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
}


/**
 * Close the DXF file.
 *
 * Safe to call if no file was opened, or opening a file failed.
 */
void IADxfWriter::close()
{
    if (pFile!=nullptr) {
        fputs(
              "0\r\n"
              "ENDSEC\r\n"
              "0\r\n"
              "EOF\r\n", pFile);

        fclose(pFile);
        pFile = nullptr;
    }
}




