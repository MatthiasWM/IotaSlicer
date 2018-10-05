//
//  Iota.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <memory>

#include "app/IAError.h"
#include "geometry/IAMesh.h"
#include "geometry/IASlice.h"
#include "printer/IAPrinterList.h"
#include "app/IAPreferences.h"


class IAGeometryReader;
class IAToolpath;
class IAMesh;
class IAPrinter;



/** Version string for this version of Iota: X.Y.XA, where X is the major
 * relase, Y is the minor release, Z is the build number, and A is an additional
 * string, 'a' for alpha versions, 'b' for beta releases, or '' for no
 * further description.
 */
extern const char *gVersion;

/**
 * The resolution of all frambuffer objects.
 * 1024 is pretty much the minimum. 2048 brings ok results. At 4096,
 * toolpaths are really nice, but rendering time slows down considerably.
 * At higher resolutions, toolpaths are perfect, but rendering time and
 * memory usage explode.
 *
 * \todo nothing is optimized here yet. There is a great potential for
 *        accelerating, manly potrace, and reduced memory allocation by
 *        smart clipping.
 * \todo draw scene viewer with antaliasing!
 */
extern const int kFramebufferSize;

/**
 * temp kludge
 * \todo these are currently only for testing textures, but should be removed.
 */
const int IA_PROJECTION_FRONT       = 0;
const int IA_PROJECTION_CYLINDRICAL = 1;
const int IA_PROJECTION_SPHERICAL   = 2;


/**
 * The main class that manages this app.
 *
 * \todo generally make class method names more consistent
 * \todo make high level functions automatically execute lower rank functions,
 *       if they were not run yet
 * \todo create a model class that contains one or more meshes
 * \todo allow multiple meshes in a scene
 * \todo assign color methods to meshes and models
 * \todo add a positional vertex (done) and a normal (still to do) for slicing
 *       in the printer coordinate system
 */
class IAIota
{
public:
    IAIota();
    ~IAIota();

    // -------- main menu callbacks
    // ---- menu File
    void userMenuFileNewProject();
    void userMenuFileOpen();
    // - open recent
    void userMenuOpenRecentFile(int ix);
    void userMenuClearRecentFileList();
    // - save project
    // - save project as
    // - print 2d
    void userMenuFileQuit();
    // ---- menu Edit
    // - ..
    // ---- menu View
    // - ..
    // ---- menu Slice
    // - run
    void userMenuSliceSave();
    void userMenuSliceSaveAs();
    void userMenuSliceClean();
    void userMenuSliceSliceAll();
    // - slice selected
    // ---- menuSettings
    void userMenuSettingsManagePrinter();
    // ---- menu Help
    void userMenuHelpVersioneer();
    void userMenuHelpAbout();

    // ---- main window
    void userMainSelectPrinter();

    // -------- settings dialog callbacks
    void userDialogSettingPrinterSelect();
    void userDialogSettingPrinterDeselect();
    void userDialogSettingPrinterAdd();
    void userDialogSettingPrinterRemove();

    // --------
    void loadDemoFiles();
    void loadAnyFile(const char *list);

    /** Set, clear, and show error messages. */
    IAError Error;


private:
    bool addGeometry(const char *name, uint8_t *data, size_t size);
    bool addGeometry(const char *filename);
    bool addGeometry(std::shared_ptr<IAGeometryReader> reader);

public:
    /// the main UI window
    /// \todo the UI must be managed in a UI class (Fluid can do that!)
    class Fl_Window *gMainWindow = nullptr;
    /// the one and only texture we currently support
    /// \todo move this into a class and attach it to models
    class Fl_RGB_Image *texture = nullptr;
    /// the one and only mesh we currently support
    /// \todo move meshes into a model class, and models into a modelList
    IAMesh *pMesh = nullptr;
    /// the current 3d printer
    IAPrinter *pCurrentPrinter = nullptr;
    /// a list of available printers
    IAPrinterList pCustomPrinterList;
    /// a list of available printer types
    IAPrinterList pPrinterPrototypeList;
    /// show the slice in the 3d view
    /// \todo move to UI class
    bool gShowSlice;
    /// show the texture in the 3d view
    /// \todo move to UI class
    bool gShowTexture;
    /// User settings for this app.
    IAPreferences gPreferences;

    int pUserDialogSettingsSelectedPrinterIndex = 0;

};


extern IAIota Iota;


#endif /* MAIN_H */
