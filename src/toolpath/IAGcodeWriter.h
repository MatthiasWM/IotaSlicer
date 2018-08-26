//
//  IAGcodeWriter.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_GCODE_WRITER_H
#define IA_GCODE_WRITER_H


#include "geometry/IAVector3d.h"

#include <vector>
#include <map>


/**
 * Helps the toolpath classes to write GCode
 */
class IAGcodeWriter
{
public:
    IAGcodeWriter();
    ~IAGcodeWriter();

    bool open(const char *filename);
    void close();

    IAVector3d &position() { return pPosition; }

    void sendMoveTo(IAVector3d &v);
    void sendRapidMoveTo(IAVector3d &v);
    void sendPosition(IAVector3d &v);
    void sendFeedrate(double f);
    void sendExtrusionAdd(double e);
    void sendExtrusionRel(uint32_t color, double e);
    void sendNewLine(const char *comment=nullptr);

    void macroInit();
    void macroShutdown();
    void macroPurgeExtruder(int t);

    void cmdHome();
    void cmdResetExtruder();
    void cmdSelectExtruder(int);
    void cmdExtrude(double distance, double feedrate=-1.0);
    void cmdExtrudeRel(double distance, double feedrate=-1.0);
    void cmdComment(const char *format, ...);
    void cmdRapidMove(double x, double y);
    void cmdRapidMove(IAVector3d &v);
    void cmdMove(double x, double y, double feedrate=-1.0);
    void cmdMove(IAVector3d &v, double feedrate=-1.0);
    void cmdMove(IAVector3d &v, uint32_t color, double feedrate=-1.0);

private:
    FILE *pFile = nullptr;
    IAVector3d pPosition;
    int pT = 0;
    double pE = 0.0;
    double pF = 0.0;
    double pRapidF = 5400.0;
    double pPrintingF = 1800.0;
    double pLayerHeight = 0.3;
    double pEFactor = 22.0;
};


#endif /* IA_GCODE_WRITER_H */


