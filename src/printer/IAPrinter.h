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
    IAPrinter();
    IAPrinter(IAPrinter const& src);
    virtual ~IAPrinter();
    IAPrinter &operator=(IAPrinter&) = delete;
    virtual IAPrinter *clone() const = 0;
    virtual const char *type() const = 0;

    // ---- direct user interaction
    virtual void userSliceSave() = 0;
    virtual void userSliceSaveAs() = 0;
    virtual void userSliceGenerateAll() = 0;

    // ----
    virtual void draw();
    virtual void drawPreview(double lo, double hi);
    virtual void purgeSlicesAndCaches();

    // ---- properties
    virtual void initializePrinterProperties();
    void buildPropertiesUI(Fl_Tree*);
    void loadProperties();
    void saveProperties();
    void removeProperties();
    void setName(const char *name);
    const char *name() const;
    void setRecentUploadFilename(const char *name);
    const char *recentUploadFilename() const;
    const char *uuid() const { return pUUID; }
    void setNewUUID();
    void setUUID(const char *uuid);

    IASettingList pPrinterProperties;
    char *pUUID = nullptr;
    char *pName = nullptr;
    char *pRecentUploadFilename = nullptr;
    bool pFirstWrite = true;
    IAVector3d pBuildVolumeMin = { 0.0, 0.0, 0.0 };
    IAVector3d pBuildVolumeMax = { 214.0, 214.0, 230.0 };
    IAVector3d pBuildVolume = { 214.0, 214.0, 230.0 };
    double pBuildVolumeRadius = 200.0; // sphere that contains the entire centered build volume

    // ---- scene settings
    virtual void initializeSceneSettings();
    void buildSessionSettings(Fl_Tree*);
    double layerHeight() { return pLayerHeight; } // Session Setting

    IASettingList pSceneSettings;
    double pLayerHeight = 0.3; // Session setting

    // ----

    /// This is the current slice that contains the entire scene at a give z.
    IASlice gSlice = IASlice(this);

protected:
    bool queryOutputFilename(const char *title,
                             const char *filter,
                             const char *extension);

private:
    void userChangedLayerHeight();

};


#endif /* IA_PRINTER_H */


