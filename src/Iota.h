//
//  Iota.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <memory>

#include "geometry/IAMesh.h"
#include "geometry/IASlice.h"
#include "printer/IAPrinter.h"
#include "userinterface/IAPreferences.h"


class IAGeometryReader;
class IAMachineToolpath;
class IAToolpath;


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
 * List of errors that the user may encounter.
 */
enum class Error {
    NoError = 0,
    CantOpenFile_STR_BSD,
    UnknownFileType_STR,
    FileContentCorrupt_STR,
	OpenGLFeatureNotSupported_STR,
};


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

    void menuNewProject();
    void menuOpen();
    void menuQuit();
    void menuSliceAs();
    void menuSliceAgain();

    void loadDemoFiles();
    void loadAnyFile(const char *list);

    void clearError();
    void setError(const char *loc, Error err, const char *str=nullptr);
    bool hadError();
    Error lastError() { return pError; }
    void showError();

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
    IAPrinterList pPrinterList;
    /// show the slice in the 3d view
    /// \todo move to UI class
    bool gShowSlice;
    /// show the texture in the 3d view
    /// \todo move to UI class
    bool gShowTexture;
    /// User settings for this app.
    IAPreferences gPreferences;

private:
    /// user definable string explaining the details of an error
    const char *pErrorString = nullptr;
    /// user definable string explaining the function that caused an error
    const char *pErrorLocation = nullptr;
    /// current error condition
    Error pError = Error::NoError;
    /// system specific error number of the call that was markes by an error
    int pErrorBSD = 0;
    /// list of error messages
    static const char *kErrorMessage[];
};


extern IAIota Iota;


#endif /* MAIN_H */
