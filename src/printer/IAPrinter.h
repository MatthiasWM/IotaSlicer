//
//  IAPrinter.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_PRINTER_H
#define IA_PRINTER_H


#include "geometry/IAVector3d.h"
#include "geometry/IASlice.h"
#include "toolpath/IAToolpath.h"
#include "IASetting.h"


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
    // ---- constructor, destructor
    IAPrinter(const char *name);
    virtual ~IAPrinter();

    // ---- direct user interaction
    virtual void userSliceSave() = 0;
    virtual void userSliceSaveAs() = 0;
    virtual void userSliceGenerateAll() = 0;


    // ----
    virtual void draw();
//    virtual void drawPreview();
    virtual void drawPreview(double lo, double hi);
    virtual void purgeSlicesAndCaches();

    void loadSettings();
    void saveSettings();

    double layerHeight() { return pLayerHeight; } // Session Setting
    
    void setName(const char *name);
    const char *name();
    void setOutputPath(const char *name);
    const char *outputPath();

    void buildSessionSettings();

    IAVector3d pBuildVolume = { 214.0, 214.0, 230.0 };
    IAVector3d pBuildVolumeMin = { 0.0, 0.0, 0.0 };
    IAVector3d pBuildVolumeMax = { 214.0, 214.0, 230.0 };
    double pBuildVolumeRadius = 200.0; // sphere that contains the entire centered build volume

    /// the toolpath for the entire scene for vector based machines
    IAMachineToolpath pMachineToolpath;

    /// This is the current slice that contains the entire scene at a give z.
    IASlice gSlice;

protected:
    bool queryOutputFilename(const char *title,
                             const char *filter,
                             const char *extension);

    bool pFirstWrite = true;

    IASettingList pSettingList;

private:
    void userChangedLayerHeight();
    
    double pLayerHeight = 0.3; // Session setting

    char *pName = nullptr;

    char *pOutputPath = nullptr;
};


#endif /* IA_PRINTER_H */


