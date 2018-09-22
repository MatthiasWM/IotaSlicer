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
    /** \todo save and update pEFactor */
    void setFilamentDiameter(double d);
    /** \todo save and update pEFactor */
    void setLayerHeight(double d);

    /** Position of the extruder head.
     \return the position of the current extruder's tip. */
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
    void cmdRetract(double d=1.0);
    void cmdUnretract(double d=1.0);

private:
    FILE *pFile = nullptr;
    IAVector3d pPosition;
    int pT = 0;
    double pE = 0.0;
    double pF = 0.0;
    double pRapidF = 5000.0;
    double pPrintingF = 1400.0;
    double pLayerHeight = 0.3;

    /*
     Calculating the E-Factor:
     When sending a G1 command, X, Y and Z give the distance of the move in mm.
     E is the ditance in mm that the cold filament will be moved.
     If the E factor was 1, and there was no hotend, G1 would extrude exactly
     the length of filament corresponding to the XYZ motion.

     So, the incoming filament is 1.75mm in diameter. That gives an area
     of PI*r^2 = (1.75/2)^2*pi = 2.41 . With a nozzle diameter of 0.4mm and
     a layer heigt of 0.3mm, we want an area of 0.4*0.3 = 0.12 .
     Dividing the surface areas 2.41/0.12 gives us the factor 20.0 .

     For a layer height of 0.4, this would be 2.41/0.16 = 15.0 .

     The lower the factor, the more material is extruded.

     I am assuming that a printer with a 0.4mm nozzle can not generate
     extrusions that are less wide or wider. Height however varies with
     layer height. This may actually not be true because hot filament
     may stretch or squash.
     */

    double pEFactor = 20.0;
};


#endif /* IA_GCODE_WRITER_H */


