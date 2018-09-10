//
//  IAPrinter.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_PRINTER_H
#define IA_PRINTER_H


#include "geometry/IAVector3d.h"

#include "geometry/IASlice.h"

#include <vector>


class Fl_Menu_Item;
class IAMachineToolpath;


/**
 * Base class to manage different types of 3D printers.
 *
 * Some terminology form CNC:
 * - worktable
 * - workpiece
 * - part program
 * - multiple workpiece setup
 *
 * \todo When printing, we generate several layers (IAPrinter should have an
 *      array of IALayer). Every layer holds an IASlice to create the color
 *      information and the volume per slice (two IAFramebuffers)
 * \todo Printer specifix data is generated from the two framebuffers. This
 *      could be images or toolpaths, or both.
 * \todo Any layer can request data from any other layer, which is either
 *      buffered or generated when needed.
 */
class IAPrinter
{
public:
    IAPrinter(const char *name);
    virtual ~IAPrinter();
    virtual void draw();
    virtual void drawPreview();
    virtual void clearHashedData();

    void loadSettings();
    void saveSettings();
    
    void setName(const char *name);
    const char *name();
    void setOutputPath(const char *name);
    const char *outputPath();

    virtual void userSliceAs();
    virtual void userSliceAgain();
    virtual void sliceAndWrite(const char *filename=nullptr);

    virtual void buildSessionSettings();

    IAVector3d pBuildVolume = { 214.0, 214.0, 230.0 };
    IAVector3d pBuildVolumeMin = { 0.0, 0.0, 0.0 };
    IAVector3d pBuildVolumeMax = { 214.0, 214.0, 230.0 };
    double pBuildVolumeRadius = 200.0; // sphere that contains the entire centered build volume

    /// the toolpath for the entire scene for vector based machines
    IAMachineToolpath *pMachineToolpath = nullptr;

    /// This is the current slice that contains the entire scene at a give z.
    IASlice gSlice;

protected:
    bool queryOutputFilename(const char *title,
                             const char *filter,
                             const char *extension);

    bool pFirstWrite = true;

private:
    char *pName = nullptr;

    char *pOutputPath = nullptr;
};


/**
 * Manage a list of printers.
 */
class IAPrinterList
{
public:
    IAPrinterList(Fl_Menu_Item *printermenu);
    ~IAPrinterList();
    bool add(IAPrinter *printer);
    IAPrinter *defaultPrinter();
    void userSelectsPrinter(IAPrinter *p);

private:
    void buildMenuArray();
    void buildSessionSettings(IAPrinter *p);
    static void userSelectsPrinterCB(Fl_Menu_Item*, void *p);

    Fl_Menu_Item *pMenuItem = nullptr;
    Fl_Menu_Item *pMenuArray = nullptr;

    std::vector<IAPrinter *> pPrinterList;
};


//class IAPrintLayer
//{
//public:
// we need the slice only once to create the color and property buffer
// there is no reason to keep it around
// the printer superclass holds this list of framebuffers
// the specialized printer class holds printer specific data, like toolpaths
//
//    IASlice *pSlice;
//    IAFramebuffer *pColorBuffer; // or use alpha buffer bit: shell, infill, support, lid, bottom
//    // also contains a 24 bit depth buffer and possibly 8 bits for masking
//    IAFramebuffer *pPropsBuffer; // properties: model shell, model infill, support, etc.
//    IAToolpath *pToolpath; // only for certain printers...
//};

#endif /* IA_PRINTER_H */


