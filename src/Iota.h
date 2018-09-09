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


extern const char *gVersion;

// temp kludge
const int IA_PROJECTION_FRONT       = 0;
const int IA_PROJECTION_CYLINDRICAL = 1;
const int IA_PROJECTION_SPHERICAL   = 2;

const int kFramebufferSize = 4096;


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
 * \todo assign color methids to meshes and models
 * \todo add a positional vertex (done) and a normal (still to do) for slicing
 *       in the printer coordinate system
 * \todo create vector infills from slices
 * \todo results get good with a framebuffer size of 4096x4096! But potrace
 *       gets slow!
 */
class IAIota
{
public:
    IAIota();
    ~IAIota();

    void sliceMesh(const char *filename);

    void menuWriteSlice();
    void menuSliceMesh();
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
    /// the one slice that we generate
    /// \todo create a current slice and hashed slices for other z-layers
    IASlice gMeshSlice;
    /// the current 3d printwer
    IAPrinter *pCurrentPrinter = nullptr;
    IAPrinterList pPrinterList;
    /// the toolpath for the entire scene
    IAMachineToolpath *pMachineToolpath = nullptr;
    /// the current toolpath
    IAToolpath *pCurrentToolpath = nullptr;
    /// show the slice in the 3d view
    /// \todo move to UI class
    bool gShowSlice;
    /// show the texture in the 3d view
    /// \todo move to UI class
    bool gShowTexture;

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
