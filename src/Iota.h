//
//  Iota.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include "geometry/IAMesh.h"
#include "geometry/IASlice.h"
#include "printer/IAPrinter.h"

class IAGeometryReader;

// TODO globals that we want to get rid of.

const int IA_PROJECTION_FRONT       = 0;
const int IA_PROJECTION_CYLINDER    = 1;
const int IA_PROJECTION_SPHERE      = 2;

/**
 * List of errors that the user may encounter.
 */
enum class Error {
    NoError = 0,
    CantOpenFile_STR_BSD,
    UnknownFileType_STR,
    FileContentCorrupt_STR,
};


/**
 * The main class that manages this app.
 */
class IAIota
{
public:
    IAIota();
    ~IAIota();

    bool addGeometry(const char *name, uint8_t *data, size_t size);
    bool addGeometry(const char *filename);
    bool addGeometry(std::shared_ptr<IAGeometryReader> reader);

private:
    bool addGeomtery(class IAGeometryReader *reader);


public: // TODO: deprecated globals are now members, but must be removed
    class Fl_Window *gMainWindow = nullptr;
    class Fl_RGB_Image *texture = nullptr;
    IAMesh *pMesh = nullptr;
    IASlice gMeshSlice;
    IAPrinter gPrinter;
    bool gShowSlice;
    bool gShowTexture;
    FILE *gOutFile = nullptr;

    void loadAnyFileList(const char *list);
    void sliceAll();
    void menuWriteSlice();
    void menuQuit();

    void clearError();
    void setError(const char *loc, Error err, const char *str=nullptr);
    bool hadError();
    Error lastError() { return pError; }
    void showError();

private:
    const char *pErrorString = nullptr;
    const char *pErrorLocation = nullptr;
    Error pError = Error::NoError;
    int pErrorBSD = 0;
    static const char *kErrorMessage[];
};


extern IAIota Iota;


#endif /* MAIN_H */
