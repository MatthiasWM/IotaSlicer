//
//  IAGcodeWriter.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAGcodeWriter.h"

#include "Iota.h"

#include <FL/gl.h>

#include <math.h>
#include <stdarg.h>


/**
 * Create a new GCode writer.
 */
IAGcodeWriter::IAGcodeWriter()
{
}


/**
 * Release all resources.
 */
IAGcodeWriter::~IAGcodeWriter()
{
    if (pFile!=nullptr) close();
}


/**
 * Open a file for wrinting GCode commands.
 *
 * \param filename destination file
 *
 * \return true, if we were able to create that file.
 *
 * \todo add user-friendly error messages.
 */
bool IAGcodeWriter::open(const char *filename)
{
    pFile = fopen(filename, "wb");
    if (!pFile) {
        // set error
        printf("Can't open file %s\n", filename);
        return false;
    }
    return true;
}


/**
 * Send the end-of-line character.
 *
 * \param comment an optional commant that is added to the line.
 */
void IAGcodeWriter::sendNewLine(const char *comment)
{
    if (comment)
        fprintf(pFile, " ; %s", comment);
    fprintf(pFile, "\n");
}


/**
 * Send the command to home the printer on all axes.
 */
void IAGcodeWriter::cmdHome()
{
    fprintf(pFile, "G28 ; home all axes\n");
    pPosition = { 0.0, 0.0, 0.0 };
}


/**
 * Send the command to retract the filament in the current extruder.
 *
 * \param d optional retraction length
 *
 * \todo the parameter d does not do much.
 */
void IAGcodeWriter::cmdRetract(double d)
{
#if 0
    // repetier long retract and short retract
    if (d>1.0)
        fprintf(pFile, "G10 S1\n");
    else
        fprintf(pFile, "G10\n");
#elif 0
    w.cmdExtrudeRel(-d);
#else
    fprintf(pFile, "G10\n");
#endif
}


/**
 * Send the command to unretract the filament in the current extruder.
 *
 * \param d optional retraction length
 *
 * \todo the parameter d does not do much.
 * \todo d should be the same d that was used when retracting. We could
 *      remember that ourselves.
 */
void IAGcodeWriter::cmdUnretract(double d)
{
#if 0
    // repetier long retract and short retract
    if (d>1.0)
        fprintf(pFile, "G11 S1\n");
    else
        fprintf(pFile, "G11\n");
#elif 0
    w.cmdExtrudeRel(d);
#else
    fprintf(pFile, "G11\n");
#endif
}


/**
 * Send the command to select another (virtual) extruder.
 *
 * \param n index of the new extruder
 *
 * \todo we need to save some parameters on a per-extruder base and update
 *      them to whichever extruder we choose.
 */
void IAGcodeWriter::cmdSelectExtruder(int n)
{
    fprintf(pFile, "T%d ; select extruder\n", n);
}


/**
 * Send the command to extrude some material, but don't move the head.
 *
 * \param distance length of filamant in mm
 * \param feedrate speed of extrusion in mm/minute
 *
 * \todo no need to send feedrate if it did not change
 * \todo we need a different pE for each extruder
 */
void IAGcodeWriter::cmdExtrude(double distance, double feedrate)
{
    if (feedrate<0.0) feedrate = pPrintingF;
    fprintf(pFile, "G1 E%.4f F%.4f\n", pE+distance, feedrate);
    pE += distance;
    pF = feedrate;
}


/**
 * Send the command to extrude some material, but don't move the head.
 *
 * \note This is for mixing extruders on Duet controllers only!
 *
 * \param distance length of filamant in mm
 * \param feedrate speed of extrusion in mm/minute
 *
 * \todo no need to send feedrate if it did not change
 * \todo we need to verify that the extrude is in relative mode
 */
void IAGcodeWriter::cmdExtrudeRel(double distance, double feedrate)
{
    if (feedrate<0.0) feedrate = pPrintingF;
    fprintf(pFile, "G1 E%.4f:%.4f:%.4f:%.4f F%.4f\n", distance/4.0, distance/4.0, distance/4.0, distance/4.0, feedrate);
    pF = feedrate;
}


/**
 * Move the printhead to a new position in current z without extruding.
 *
 * \param x, y new position in mm from origin.
 */
void IAGcodeWriter::cmdRapidMove(double x, double y)
{
    IAVector3d p(x, y, pPosition.z());
    cmdRapidMove(p);
}


/**
 * Move the printhead to a new position without extruding.
 *
 * \param v new position in mm from origin in x, y, and z.
 */
void IAGcodeWriter::cmdRapidMove(IAVector3d &v)
{
    sendRapidMoveTo(v);
    sendFeedrate(pRapidF);
    sendNewLine();
}


/**
 * Move the printhead to a new position in current z while extruding.
 *
 * \param x, y new position in mm from origin.
 * \param feedrate this is actually the extrusion lentgh divided by the length
 *      of the vector
 *
 * \todo What exactly is the feedrate parameter now, and could we simplify that?
 */
void IAGcodeWriter::cmdMove(double x, double y, double feedrate)
{
    IAVector3d p(x, y, pPosition.z());
    cmdMove(p, feedrate);
}


/**
 * Move the printhead to a new position while extruding.
 *
 * \param v new position in mm from origin in x, y, and z.
 * \param feedrate this is actually the extrusion lentgh divided by the length
 *      of the vector
 *
 * \todo What exactly is the feedrate parameter now, and could we simplify that?
 */
void IAGcodeWriter::cmdMove(IAVector3d &v, double feedrate)
{
    if (feedrate<0.0) feedrate = pF;
    double len = (v-pPosition).length();
    sendMoveTo(v);
    sendExtrusionAdd(len/pEFactor);
    sendFeedrate(feedrate);
    sendNewLine();
}


/**
 * Move the printhead to a new position while extruding.
 *
 * \param v new position in mm from origin in x, y, and z.
 * \param color packed color, assuming mixing extruder
 * \param feedrate this is actually the extrusion lentgh divided by the length
 *      of the vector
 *
 * \todo What do color and feedrate do?
 */
void IAGcodeWriter::cmdMove(IAVector3d &v, uint32_t color, double feedrate)
{
    if (feedrate<0.0) feedrate = pF;
    double len = (v-pPosition).length();
    sendMoveTo(v);
    sendExtrusionRel(color, len/pEFactor);
    sendFeedrate(feedrate);
    sendNewLine();
}


/**
 * Send all commands to initialize a specific machine.
 *
 * \todo This should be more general, plus some commands for specific machines.
 */
void IAGcodeWriter::macroInit()
{
#ifdef IA_QUAD
#error
    fprintf(pFile, "; generated by Iota Slicer\n");
    cmdComment("");
    cmdComment("==== Macro Init");
    fprintf(pFile, "G21 ; set units to millimeters\n");
    fprintf(pFile, "G90 ; use absolute coordinates\n");
    fprintf(pFile, "G28 ; home all axes\n");
    fprintf(pFile, "G1 Z5 F5000 ; lift nozzle\n");
    fprintf(pFile, "M140 S60 ; set bed temperature\n");
    fprintf(pFile, "M563 P0 D0:1:2:3 H0 F0 ; set tool 0 as full color on quad\n");
    fprintf(pFile, "T0\n");
    // Using relative distances for extrusion is a very bad idea, but we nned to know how the Quad board behave before we can fix this
    fprintf(pFile, "M83 ; use relative distances for extrusion\n");
    fprintf(pFile, "M104 S230 ; set extruder temperature\n");
    fprintf(pFile, "M109 S230 ; set temperature and wait for it to be reached\n");
    fprintf(pFile, "M190 S60 ; wait for bed temperature\n");
    cmdResetExtruder();
    fprintf(pFile, "G1 F800 E3:0:0:0 ; purge\n");
    fprintf(pFile, "G1 F800 E0:3:0:0 ; purge\n");
    fprintf(pFile, "G1 F800 E0:0:3:0 ; purge\n");
    fprintf(pFile, "G1 F800 E0:0:0:3 ; purge\n");
    fprintf(pFile, "M106 S255 P0 ; fan on\n");
    fprintf(pFile, "M106 S255 P1 ; fan on\n");
    fprintf(pFile, "M106 S255 P2 ; fan on\n");
    fprintf(pFile, "G4 S0.1 ; dwell\n");
    // C=523.251, D=587.330, E=659.255, F=698.456, G=783.991, A=880, B=987.767, C=1046.50
    fprintf(pFile, "M300 S523.251 P100 ; beep\n");
    fprintf(pFile, "M300 S587.330 P100 ; beep\n");
    fprintf(pFile, "M300 S659.255 P100 ; beep\n");
    fprintf(pFile, "M300 S698.456 P100 ; beep\n");
    fprintf(pFile, "M300 S783.991 P100 ; beep\n");
#else
    fprintf(pFile, "; generated by Iota Slicer\n");
    cmdComment("");
    cmdComment("==== Macro Init");
    fprintf(pFile, "G21 ; set units to millimeters\n");
    fprintf(pFile, "G90 ; use absolute coordinates\n");
    fprintf(pFile, "G28 ; home all axes\n");
    fprintf(pFile, "G1 Z5 F5000 ; lift nozzle\n");
    fprintf(pFile, "M140 S60 ; set bed temperature\n");
//    fprintf(pFile, "T1\n");
//    fprintf(pFile, "M82 ; use absolute distances for extrusion\n");
//    fprintf(pFile, "M104 S230 ; set extruder temperature\n");
    fprintf(pFile, "T0\n");
    fprintf(pFile, "M82 ; use absolute distances for extrusion\n");
    fprintf(pFile, "M104 S230 ; set extruder temperature\n");
    fprintf(pFile, "M109 S230 ; set temperature and wait for it to be reached\n");
    fprintf(pFile, "M190 S60 ; wait for bed temperature\n");
    cmdResetExtruder();
    sendMoveTo(pPosition); sendExtrusionAdd(-1.0); sendFeedrate(1800.0); sendNewLine("retract extruder");
    fprintf(pFile, "M106 S255 P0 ; fan on\n");
    fprintf(pFile, "M106 S255 P1 ; fan on\n");
    fprintf(pFile, "M106 S255 P2 ; fan on\n");
    fprintf(pFile, "G4 S0.1 ; dwell\n");
    // C=523.251, D=587.330, E=659.255, F=698.456, G=783.991, A=880, B=987.767, C=1046.50
    fprintf(pFile, "M300 S523.251 P100 ; beep\n");
    fprintf(pFile, "M300 S587.330 P100 ; beep\n");
    fprintf(pFile, "M300 S659.255 P100 ; beep\n");
    fprintf(pFile, "M300 S698.456 P100 ; beep\n");
    fprintf(pFile, "M300 S783.991 P100 ; beep\n");
#if 0
    sendMoveTo(pPosition);
    sendExtrusionAdd(8);
    sendFeedrate(1800);
    sendNewLine("purge some filament");
#elif 1
    // do nothing
#else
    macroPurgeExtruder(0);
    macroPurgeExtruder(1);
#endif
    IAVector3d v = { 0.0, 0.0, 0.2 };
    cmdRapidMove(v);
    cmdComment("");
#endif
}


/**
 * Send the commands needed to end printing.
 *
 * \todo This should be more general, plus some commands for specific machines.
 */
void IAGcodeWriter::macroShutdown()
{
    cmdComment("");
    cmdComment("==== Macro Shutdown");
    cmdResetExtruder();
//    fprintf(pFile, "T1 M104 S0 ; set extruder temperature\n");
    fprintf(pFile, "T0\n");
    fprintf(pFile, "M104 S0 ; set extruder temperature\n");
    fprintf(pFile, "M140 S0 ; set bed temperature\n");
    fprintf(pFile, "M106 S0 P0 ; fan off\n");
    fprintf(pFile, "M106 S0 P1 ; fan off\n");
    fprintf(pFile, "M106 S0 P2 ; fan off\n");
    fprintf(pFile, "G28 X0 Y0  ; home X and Y axis\n");
    fprintf(pFile, "M84 ; disable motors\n");

    fprintf(pFile, "G4 S0.1 ; dwell\n");
    fprintf(pFile, "M300 S783.991 P100 ; beep\n");
    fprintf(pFile, "M300 S698.456 P100 ; beep\n");
    fprintf(pFile, "M300 S659.255 P100 ; beep\n");
    fprintf(pFile, "M300 S587.330 P100 ; beep\n");
    fprintf(pFile, "M300 S523.251 P100 ; beep\n");
    cmdComment("");
}


/**
 * Send the commands to purge some extruder and make it ready for printing.
 *
 * \param t tool number for the extruder that we want to purge.
 *
 * \todo This should be more general, plus some commands for specific machines.
 */
void IAGcodeWriter::macroPurgeExtruder(int t)
{
    cmdComment("");
    cmdComment("==== Macro Purge Extruder %d", t);
    if (t==0) {
        // The tool 0 box is mounted in a very bad place and can just barely be reached.
        // We need to drive the head in a circle to actually wipe, and make a bee line
        // so that waste goes into the waste box. Phew. Also, when usug repetier FW, the
        // width needs to be corrected so that we reach the waste container at all.
        // Wipe tool 0
        // Box:     200, 0, 10
        // Wipe:    180, 0, 10
        cmdRapidMove(90,  0);
        cmdSelectExtruder(t);
        cmdResetExtruder();
        cmdRapidMove(180,  0);
        cmdRapidMove(180, 20);
        cmdRapidMove(199, 20);
        cmdRapidMove(199,  0);
        cmdRapidMove(180,  0);
        cmdRapidMove(180, 20);
        cmdRapidMove(199, 20);
        cmdRapidMove(199,  0);
        cmdExtrude(30);
        cmdRapidMove(180,  0);
        cmdRapidMove(180, 20);
        cmdRapidMove(199, 20);
        cmdRapidMove(199,  0);
        cmdRapidMove(180,  0);
        cmdRapidMove(180, 20);
        cmdRapidMove(199, 20);
        cmdRapidMove(199,  0);
        cmdRapidMove(180,  0);
        cmdRapidMove(180, 20);
        cmdResetExtruder();
    } else if (t==1) {
        // The tool 1 box is located a little bit better, but in order to reach it,
        // tool 0 must be active. I am not even sure if we can send a command to extrude
        // to head 1 without changing the position back onto the bed. In effect, we would
        // have change the minimum position for x to minus whatever the offest is between heads.
        // Maybe that's why in the original firmware, the *right* head is tool 0, and
        // the *left* head ist tool 1?
        // Wipe tool 1
        // Box:     T0! 0, 0, 10
        // Wipe:    T0! 0, 20, 10
        // but can we extrude T1 without changing the position? Maybe by using relative
        // positioning?
        cmdRapidMove(90,  0);
        cmdSelectExtruder(t);
        cmdResetExtruder();
        cmdRapidMove( 0,  0);
        cmdRapidMove( 0, 20);
        cmdRapidMove(20, 20);
        cmdRapidMove(20,  0);
        cmdRapidMove( 0,  0);
        cmdExtrude(30);
        cmdRapidMove( 0, 20);
        cmdRapidMove(20, 20);
        cmdRapidMove(20,  0);
        cmdRapidMove( 0,  0);
        cmdRapidMove( 0, 20);
        cmdRapidMove(20, 20);
        cmdRapidMove(20,  0);
        cmdRapidMove( 0,  0);
        cmdRapidMove( 0, 20);
        cmdRapidMove(20, 20);
        cmdRapidMove(20,  0);
        cmdResetExtruder();
    }
    cmdComment("");
}


/**
 * Send the 'move' component of a GCode command.
 */
void IAGcodeWriter::sendMoveTo(IAVector3d &v)
{
    fprintf(pFile, "G1 ");
    sendPosition(v);
}


/**
 * Send the 'rapid move' component of a GCode command.
 */
void IAGcodeWriter::sendRapidMoveTo(IAVector3d &v)
{
    fprintf(pFile, "G0 ");
    sendPosition(v);
}


/**
 * Send the x, y, and z component of a GCode command.
 */
void IAGcodeWriter::sendPosition(IAVector3d &v)
{
    if (v.x()!=pPosition.x())
        fprintf(pFile, "X%.3f ", v.x());
    if (v.y()!=pPosition.y())
        fprintf(pFile, "Y%.3f ", v.y());
    if (v.z()!=pPosition.z())
        fprintf(pFile, "Z%.3f ", v.z());
    pPosition = v;
}


/**
 * Send the feedrate component of a GCode command.
 */
void IAGcodeWriter::sendFeedrate(double f)
{
    if (f!=pF) { fprintf(pFile, "F%.1f ", f); pF = f; }
}


/**
 * Send the extrusion component of a GCode command.
 *
 * \param e relative extrusion in mm, will be added to the total extrusion in absolute mode.
 *
 * \todo how do we know if we are in absolute or relative mode?
 */
void IAGcodeWriter::sendExtrusionAdd(double e)
{
    double newE = pE + e;
    if (newE!=pE) { fprintf(pFile, "E%.5f ", newE); pE = newE; }
}


/**
 * Send the relative extrusion component of a GCode command for a mixing extruder.
 *
 * \param color packed format: 0x00RRGGBB.
 * \param e total extrusion rate of transport r, g, b, and key.
 */
void IAGcodeWriter::sendExtrusionRel(uint32_t color, double e)
{
    double r = (double((color>>16)&255))/255.0/4.0;
    double g = (double((color>>8)&255))/255.0/4.0;
    double b = (double((color>>0)&255))/255.0/4.0;
    double k = 1.0 - r - g - b;
    fprintf(pFile, "E%.4f:%.4f:%.4f:%.4f ", r*e, g*e, b*e, k*e);
}


/**
 * Reset the current total extrusion counter.
 */
void IAGcodeWriter::cmdResetExtruder()
{
    fprintf(pFile, "G92 E0 ; reset extruder\n");
    pE = 0.0;
}



/**
 * Send a single line with a comment in printf() formatting.
 */
void IAGcodeWriter::cmdComment(const char *format, ...)
{
    fprintf(pFile, "; ");
    va_list va;
    va_start(va, format);
    vfprintf(pFile, format, va);
    va_end(va);
    fprintf(pFile, "\n");
}


/**
 * Close the GCode writer.
 */
void IAGcodeWriter::close()
{
    if (pFile!=nullptr) {
        fclose(pFile);
        pFile = nullptr;
    }
}




