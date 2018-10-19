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
#include <memory>


class IAToolpathList;
class IAToolpath;
class IAToolpathElement;
class IAFramebuffer;
class IAFDMPrinter;


typedef std::map<int, IAToolpathList*> IAToolpathListMap;
typedef std::vector<IAToolpath*> IAToolpathTypeList;
typedef std::vector<IAToolpathElement*> IAToolpathElementList;
typedef std::shared_ptr<IAToolpathList> IAToolpathListSP;
typedef std::shared_ptr<IAToolpath> IAToolpathTypeSP;


/**
 * Represents a list of commands (settings, motions) for a 3d printer.
 *
 * This is an internal storage format for commands that need to be sent
 * to a machine to create the desired 3D printout.
 *
 * This class is used to draw the toolpath into the scene view. It is also
 * the source for generating GCode commands. All coordinates in this class
 * must be kept in world space. The GCode writer must flip or offset
 * coordinates to math the printer space. Scaling should not be needed.
 *
 * \todo We don't want that anymore. Toolpaths should be part of the layer
 *      list inside each printer.
 */
class IAMachineToolpath
{
public:
    IAMachineToolpath(IAFDMPrinter *printer);
    ~IAMachineToolpath();
    void purge();
    
    void draw(double lo, double hi);
    void drawLayer(double);
    IAToolpathList *findLayer(double);
    IAToolpathList *createLayer(double);
    void deleteLayer(double);
    int roundLayerNumber(double);
    void optimize();

    bool saveGCode(const char *filename);

    unsigned int createToolmap();

private:
    IAToolpathListMap pToolpathListMap;
    IAFDMPrinter *pPrinter = nullptr;
};


/*
 Add the class IAToolpathList and use IAToolpath as a superclass for
 IAToolpathLoop and IAToolpathLine. IAToolpathList can then sort and optimize
 its Loops and Lines by priority and grouping.
 */

class IAToolpathList
{
public:
    IAToolpathList(double z);
    ~IAToolpathList();
    void purge();
    void setZ(double z) { pZ = z; }
    void draw();
    void drawFlat(double w);
    void drawFlatToBitmap(IAFramebuffer*, double w, int color=0);

    void add(IAToolpathList *tl, int tool, int group, int priority);
    void add(IAToolpath *tt, int tool, int group, int priority);

    bool isEmpty();

    void optimize();

    unsigned int createToolmap();

//    void colorize(uint8_t *rgb, IAToolpath *black, IAToolpath *white);
//    void colorizeSoft(uint8_t *rgb, IAToolpath *dst);

    void saveGCode(IAGcodeWriter &g);
    void saveDXF(const char *filename);

    IAToolpathTypeList pToolpathList;

    double pZ;
};


class IAToolpath
{
protected:
    IAToolpath(double z);
public:
    static bool comparePriorityAscending(const IAToolpath *a, const IAToolpath *b);

    virtual ~IAToolpath();
    virtual IAToolpath *clone(IAToolpath *t=nullptr);

    void purge();
    void setZ(double z) { pZ = z; tFirst.z(z); tPrev.z(z); }
    void draw();
    void drawFlat(double w);
    void drawFlatToBitmap(IAFramebuffer*, double w, int color=0);

    bool isEmpty() { return pElementList.empty(); }

    void startPath(double x, double y);
    void continuePath(double x, double y);
    void closePath(void);

//    void colorize(uint8_t *rgb, IAToolpath *black, IAToolpath *white);
//    void colorizeSoft(uint8_t *rgb, IAToolpath *dst);
    
    unsigned int createToolmap();

    void saveGCode(IAGcodeWriter &g);
    void saveDXF(IADxfWriter &w);

    IAToolpathElementList pElementList;
    // list of elements

    IAVector3d tFirst, tPrev;
    double pZ = 0.0;
    int pTool = -1;
    int pGroup = 0;
    int pPriority = 0;
};


class IAToolpathLoop : public IAToolpath
{
    typedef IAToolpath super;
public:
    IAToolpathLoop(double z);
    virtual ~IAToolpathLoop() override;
    virtual IAToolpath *clone(IAToolpath *t=nullptr) override;
};


class IAToolpathLine : public IAToolpath
{
    typedef IAToolpath super;
public:
    IAToolpathLine(double z);
    virtual ~IAToolpathLine() override;
    virtual IAToolpath *clone(IAToolpath *t=nullptr) override;
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
    virtual void drawFlat(double w) { }
    virtual void drawFlatToBitmap(IAFramebuffer*, double w, int color=0) { }
    virtual void saveGCode(IAGcodeWriter &g) { }
    virtual void saveDXF(IADxfWriter &g) { }
    virtual IAToolpathElement *clone();
};


/**
 * Element changes to another extruder.
 */
//class IAToolpathExtruder : public IAToolpathElement
//{
//public:
//    IAToolpathExtruder(int i);
//    virtual ~IAToolpathExtruder() override;
//    virtual void draw() override { }
//    virtual void drawFlat(double w) override { }
//    virtual void saveGCode(IAGcodeWriter &g) override;
//    virtual IAToolpathElement *clone() override;
//
//    int pTool = 0;
//};


/**
 * Toolpath element for extruder motion.
 */
class IAToolpathMotion : public IAToolpathElement
{
public:
    IAToolpathMotion(IAVector3d &a, IAVector3d &b, bool rapid=false);
    virtual void draw() override;
    virtual void drawFlat(double w) override;
    virtual void drawFlatToBitmap(IAFramebuffer*, double w, int color=0) override;
    virtual void saveGCode(IAGcodeWriter &g) override;
    virtual void saveDXF(IADxfWriter &g) override;
    virtual IAToolpathElement *clone() override;
    void setColor(uint32_t c) { pColor = c; }

    IAVector3d pStart, pEnd;
    uint32_t pColor = 0xFFFFFFFF;
    bool pIsRapid = false;
};


/*
 further elements could be
  - tool change
  - tool cleaning
  - machine setup, bed heating, extruder heating, etc.
 */


#endif /* IA_TOOLPATH_H */


