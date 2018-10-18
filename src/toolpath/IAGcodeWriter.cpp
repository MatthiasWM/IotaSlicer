//
//  IAGcodeWriter.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAGcodeWriter.h"

#include "Iota.h"
#include "printer/IAPrinterFDM.h"

#include <FL/gl.h>

#include <math.h>
#include <stdarg.h>


#ifdef __APPLE__
#pragma mark -
#endif
// =============================================================================


/**
 * Create a new GCode writer.
 */
IAGcodeWriter::IAGcodeWriter(IAPrinterFDM *printer)
:   pPrinter( printer )
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
    pPosition = { 0.0, 0.0, 0.0 };
    pT = -1;
    pE = 0.0;
    pF = 0.0;
    pRapidFeedrate = 3000.0;
    pPrintFeedrate = 1000.0;
    pLayerHeight = 0.3;
    pLayerStartTime = 0.0;
    pTotalTime = 0.0;
    pEFactor = ((pPrinter->filamentDiameter()/2)*(pPrinter->filamentDiameter()/2)*M_PI)
             / (pPrinter->nozzleDiameter()*pPrinter->layerHeight());
    return true;
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


#ifdef __APPLE__
#pragma mark -
#endif
// =============================================================================


/** Set the default feedrate for rapid moves. */
void IAGcodeWriter::setRapidFeedrate(double feedrate)
{
    pRapidFeedrate = feedrate;
}


/** Set the default feedrate for printing moves. */
void IAGcodeWriter::setPrintFeedrate(double feedrate)
{
    pPrintFeedrate = feedrate;
}



void IAGcodeWriter::resetTotalTime()
{
    pTotalTime = 0.0;
}


double IAGcodeWriter::getTotalTime()
{
    return pTotalTime;
}


void IAGcodeWriter::resetLayerTime()
{
    pLayerStartTime = pTotalTime;
}


double IAGcodeWriter::getLayerTime()
{
    return pTotalTime - pLayerStartTime;
}


#ifdef __APPLE__
#pragma mark -
#endif
// =============================================================================


/**
 * Send the command to home the printer on all axes.
 */
void IAGcodeWriter::cmdHome()
{
    fprintf(pFile, "G28 ; home all axes\n");
    pPosition = { 0.0, 0.0, 0.0 };
    // unknown time to execute
}


/**
 * Send the command to retract the filament in the current extruder.
 *
 * \param d optional retraction length
 *
 * \todo if d works or not depends on the GCode dialect that the printer
 *      firmware uses.
 * \todo we want to do smart retraction using continuous movement.
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
    // lowest common denominator
    fprintf(pFile, "G10\n");
    pTotalTime += 0.1; // assuming this time to execute
#endif
}


/**
 * Send the command to unretract the filament in the current extruder.
 *
 * \param d optional retraction length
 *
 * \todo if d works or not depends on the GCode dialect that the printer
 *      firmware uses.
 * \todo d should be the same d that was used when retracting. We could
 *      remember that ourselves.
 * \todo we want to do smart retraction using continuous movement.
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
    // lowest common denominator
    fprintf(pFile, "G11\n");
    pTotalTime += 0.1; // assuming this time to execute
#endif
}


/**
 * Send the command to select another (virtual) extruder.
 *
 * \param n index of the new extruder
 *
 * \todo we need to save some parameters on a per-extruder basis and update
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
 * \todo this works in absolute mode only
 */
void IAGcodeWriter::cmdExtrude(double distance, double feedrate)
{
    if (feedrate<0.0) feedrate = pPrintFeedrate;
    fprintf(pFile, "G1 E%.4f F%.4f\n", pE+distance, feedrate);
    pE += distance;  // mm
    pF = feedrate;   // mm/min
    pTotalTime += distance / (feedrate/60.0);
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
    if (feedrate<0.0) feedrate = pPrintFeedrate;
    fprintf(pFile, "G1 E%.4f:%.4f:%.4f:%.4f F%.4f\n", distance/4.0, distance/4.0, distance/4.0, distance/4.0, feedrate);
    pF = feedrate;
    pTotalTime += distance / (feedrate/60.0);
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
    sendFeedrate(pRapidFeedrate);
    sendNewLine();
    double distance = (v-position()).length();
    pTotalTime += distance / (pRapidFeedrate/60.0);
}


/**
 * Move the printhead rapidly while retracting and then unretracting.
 *
 * The goal of this function is to print without pause by retracting while
 * moving rapidly at the same time. This avoids the typical retraction pause
 * that otherwise stalls the entire print.
 *
 * \param v new position in mm from origin in x, y, and z.
 *
 * \todo What exactly is the feedrate parameter now, and could we simplify that?
 */
void IAGcodeWriter::cmdRetractMove(IAVector3d &v)
{
//    cmdComment("Retract");
#if 0
    // simple method including a pause:
    double len = (v-pPosition).length();
    bool retract = (len > 5.0);
    if (retract) cmdRetract();
    cmdRapidMove(v);
    if (retract) cmdUnretract();
#else
    // complex method:
    // - first, we move rapidly toward v while retracting the filament
    // - when the filament is retracted sufficiently, we continue to move the
    //   head without moving the filament
    // - last, we unretract the filament so that filament is available again,
    //   just as we reach the position v

    // Filament = (1.75/2)^2*pi = 2.41, Extrusion = (0.4/2)^2*pi = 0.125
    /** \todo tune this parameter */
    const double retraction = 4.0 / pEFactor; // mm filament (factor 0.05 or 20.0)
    // distance of first and last rapid motion
    double retrDist = retraction * (pRapidFeedrate/pPrintFeedrate);
    // total distance to travel
    double totalDist = (v-pPosition).length();
    IAVector3d start = pPosition;
    IAVector3d direction = (v-pPosition).normalized();

    if (2.0*retrDist>=totalDist) {
        double f = totalDist/(2.0*retrDist);
        // first move, retract while moving
        sendMoveTo(start + direction*(f*retrDist));
        sendExtrusionAdd(f*-retraction);
        sendFeedrate(pRapidFeedrate);
        sendNewLine();
        // last, move and unretract
        sendMoveTo(v);
        sendExtrusionAdd(f*retraction);
        sendFeedrate(pRapidFeedrate);
        sendNewLine();
    } else {
        // first move, retract while moving
        sendMoveTo(start + direction*retrDist);
        sendExtrusionAdd(-retraction);
        sendFeedrate(pRapidFeedrate);
        sendNewLine();
        // now just move rapidly
        sendMoveTo(start + direction*(totalDist-retrDist));
        sendFeedrate(pRapidFeedrate);
        sendNewLine();
        // last, move and unretract
        sendMoveTo(v);
        sendExtrusionAdd(retraction);
        sendFeedrate(pRapidFeedrate);
        sendNewLine();
    }
    pTotalTime += totalDist / (pRapidFeedrate/60.0);
#endif
//    cmdComment("Retract End");
}




/**
 * Move the printhead to a new position in current z while extruding.
 *
 * \param x, y new position in mm from origin.
 *
 * \todo What exactly is the feedrate parameter now, and could we simplify that?
 */
void IAGcodeWriter::cmdPrintMove(double x, double y)
{
    IAVector3d p(x, y, pPosition.z());
    cmdPrintMove(p);
}


/**
 * Move the printhead to a new position while extruding.
 *
 * \param v new position in mm from origin in x, y, and z.
 *
 * \todo What exactly is the feedrate parameter now, and could we simplify that?
 */
void IAGcodeWriter::cmdPrintMove(IAVector3d &v)
{
    double distance = (v-pPosition).length();
    sendMoveTo(v);
    sendExtrusionAdd(distance/pEFactor);
    sendFeedrate(pPrintFeedrate);
    sendNewLine();
    pTotalTime += distance / (pPrintFeedrate/60.0);
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
//void IAGcodeWriter::cmdMove(IAVector3d &v, uint32_t color, double feedrate)
//{
//    if (feedrate<0.0) feedrate = pF;
//    double len = (v-pPosition).length();
//    sendMoveTo(v);
//    sendExtrusionRel(color, len/pEFactor);
//    sendFeedrate(feedrate);
//    sendNewLine();
//}


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
 * Pause the printer for a number of seconds.
 *
 * This is used to let fine structures cool down before starting new layers.
 */
void IAGcodeWriter::cmdDwell(double seconds)
{
    fprintf(pFile, "G4 P%.f", seconds*1000);
    sendNewLine("wait");
    pTotalTime += seconds;
}

#ifdef __APPLE__
#pragma mark -
#endif
// =============================================================================


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
 * Send the 'move' component of a GCode command.
 */
void IAGcodeWriter::sendMoveTo(const IAVector3d &v)
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
void IAGcodeWriter::sendPosition(const IAVector3d &v)
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
    if (f!=pF) {
        fprintf(pFile, "F%.1f ", f);
        pF = f;
    }
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
    if (newE!=pE) {
        fprintf(pFile, "E%.5f ", newE);
        pE = newE;
    }
}


/**
 * Send the relative extrusion component of a GCode command for a mixing extruder.
 *
 * \param color packed format: 0x00RRGGBB.
 * \param e total extrusion rate of transport r, g, b, and key.
 */
//void IAGcodeWriter::sendExtrusionRel(uint32_t color, double e)
//{
//    double r = (double((color>>16)&255))/255.0/4.0;
//    double g = (double((color>>8)&255))/255.0/4.0;
//    double b = (double((color>>0)&255))/255.0/4.0;
//    double k = 1.0 - r - g - b;
//    fprintf(pFile, "E%.4f:%.4f:%.4f:%.4f ", r*e, g*e, b*e, k*e);
//}


#ifdef __APPLE__
#pragma mark -
#endif
// =============================================================================


/**
 * Send all commands to initialize a specific machine.
 *
 * \todo This should be more general, plus some commands for specific machines.
 */
void IAGcodeWriter::sendInitSequence(unsigned int toolmap)
{
    pToolmap = toolmap;
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
    if (pToolmap&1) {
        fprintf(pFile, "T0\n");
        fprintf(pFile, "M82 ; use absolute distances for extrusion\n");
        fprintf(pFile, "M104 S%d ; set extruder temperature\n", pExtruderStandbyTemp);
    }
    if (pToolmap&2) {
        fprintf(pFile, "T1\n");
        fprintf(pFile, "M82 ; use absolute distances for extrusion\n");
        fprintf(pFile, "M104 S%d ; set extruder temperature\n", pExtruderStandbyTemp);
    }
    if (pToolmap&1) {
        fprintf(pFile, "T0\n");
        fprintf(pFile, "M109 S%d ; set temperature and wait for it to be reached\n", pExtruderStandbyTemp);
    }
    if (pToolmap&2) {
        fprintf(pFile, "T1\n");
        fprintf(pFile, "M109 S%d ; set temperature and wait for it to be reached\n", pExtruderStandbyTemp);
    }
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
void IAGcodeWriter::sendShutdownSequence()
{
    cmdComment("");
    cmdComment("==== Macro Shutdown");
    cmdResetExtruder();
    //    fprintf(pFile, "T1 M104 S0 ; set extruder temperature\n");
    if (pToolmap&1) {
        fprintf(pFile, "T0\n");
        fprintf(pFile, "M104 S0 ; set extruder temperature\n");
    }
    if (pToolmap&2) {
        fprintf(pFile, "T1\n");
        fprintf(pFile, "M104 S0 ; set extruder temperature\n");
    }
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


void IAGcodeWriter::requestTool(int t)
{
    if (pT!=t) {
        // TODO: there will be a whole bunch of extruder change strategies
        // --- This is the strategy for multiple hotends with one feed each
        if (pT!=-1) {
            fprintf(pFile, "T%d\n", pT);
            fprintf(pFile, "M104 S%d ; standby temperature\n", pExtruderStandbyTemp);
            cmdRetract();
        }
        if (t!=-1) {
            // -- select the new extruder
            fprintf(pFile, "T%d\n", t);
            // -- move out of the way of the model
            IAVector3d pause = Iota.pMesh->pMin
                - IAVector3d(10.0, 10.0, 0.0)
                + Iota.pMesh->position(); /** \bug in world coordinates */
            pause.setMax(IAVector3d(0.0, 0.0, 0.0)); // FIXME: back-right of the bed (or beyond)
            pause.z( pPosition.z() );
            cmdRapidMove(pause);
            // -- wait for printing temperature
            fprintf(pFile, "M109 S%d ; printing temperature and wait\n", pExtruderPrintTemp);
            // -- or purge, or print outline, or print waste tower, or ...
            cmdUnretract();
        }
        pT = t;
    }
}


/**
 * Send the commands to purge some extruder and make it ready for printing.
 *
 * \param t tool number for the extruder that we want to purge.
 *
 * \todo This should be more general, plus some commands for specific machines.
 */
void IAGcodeWriter::sendPurgeExtruderSequence(int t)
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


