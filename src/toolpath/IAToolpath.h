//
//  IAToolpath.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_TOOLPATH_H
#define IA_TOOLPATH_H


#include "IAGcodeWriter.h"
#include "IADxfWriter.h"
#include "geometry/IAVector3d.h"

#include <vector>
#include <map>


class IAToolpath;
class IAToolpathElement;

typedef std::map<int, IAToolpath*> IAToolpathMap;
typedef std::vector<IAToolpathElement*> IAToolpathElementList;



/**
 * Represents a list commands (settings, motions) for a 3d printer.
 *
 * This is an internal storage format for commands that need to be sent
 * to a machine to create the desired 3D printout.
 *
 * This class is used to draw the toolpath into the scene view. It is also
 * the source for generating GCode commands. All coordinates in this class
 * must be kept in world space. The GCode writer must flip or offset
 * coordinates to math the printer space. Scaling should not be needed.
 */
class IAMachineToolpath
{
public:
    IAMachineToolpath();
    ~IAMachineToolpath();

    void clear();
    void draw();
    void drawLayer(double);
    IAToolpath *findLayer(double);
    IAToolpath *createLayer(double);
    void deleteLayer(double);
    int roundLayerNumber(double);

    bool saveGCode(const char *filename);

private:
    IAToolpath *pStartupPath = nullptr;
    IAToolpathMap pLayerMap;
    IAToolpath *pShutdownPath = nullptr;
};


/**
 * A toolpath, usually for a single slice, for startup, or for shutdown.
 */
class IAToolpath
{
public:
    IAToolpath(double z);
    ~IAToolpath();
    void clear(double z);
    void draw();
    void drawFlat(double w);
    void add(IAToolpath &tp);

    void startPath(double x, double y, double z);
    void continuePath(double x, double y, double z);
    void closePath(void);

    void colorize(uint8_t *rgb, IAToolpath *black, IAToolpath *white);
    void colorizeSoft(uint8_t *rgb, IAToolpath *dst);

    void saveGCode(IAGcodeWriter &g);
    void saveDXF(const char *filename);

    IAToolpathElementList pList;
    // list of elements

    IAVector3d tFirst, tPrev;
    double pZ;

};


/**
 * Base class for any element inside the tool path.
 */
class IAToolpathElement
{
public:
    IAToolpathElement();
    virtual ~IAToolpathElement();
    virtual void draw();
    virtual void drawFlat() { }
    virtual void saveGCode(IAGcodeWriter &g) { }
    virtual void saveDXF(IADxfWriter &g) { }
    virtual IAToolpathElement *clone();
};


/**
 * Element chagnes to another extruder.
 */
class IAToolpathExtruder : public IAToolpathElement
{
public:
    IAToolpathExtruder(int i);
    virtual ~IAToolpathExtruder() override;
    virtual void draw() override { }
    virtual void drawFlat() override { }
    virtual void saveGCode(IAGcodeWriter &g) override;
    virtual IAToolpathElement *clone() override;

    int pTool = 0;
};


/**
 * Toolpath element for extruder motion.
 */
class IAToolpathMotion : public IAToolpathElement
{
public:
    IAToolpathMotion(IAVector3d &a, IAVector3d &b, bool rapid=false);
    virtual void draw() override;
    virtual void drawFlat() override;
    virtual void saveGCode(IAGcodeWriter &g) override;
    virtual void saveDXF(IADxfWriter &g) override;
    virtual IAToolpathElement *clone() override;
    void setColor(uint32_t c) { pColor = c; }

    IAVector3d pStart, pEnd;
    uint32_t pColor = 0x00FFFFFF;
    bool pIsRapid = false;
};


/*
 further elements could be
  - tool change
  - tool cleanin
  - machine setup, bed heating, extruder heating, etc.
 */


#endif /* IA_TOOLPATH_H */


