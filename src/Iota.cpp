//
//  Iota.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

// TODO: fix STL importer to generate only watertight models
// TODO: make STL importer bullet proof (no reads beyond end of line)
// TODO: create a model class that contains meshes
//   TODO: add a positional vertex and a normal for slicing in the printer coordinate system
//   TODO: update slice class to generate textures and vectors in printer coordinate system
//   TODO: draw slice in 3d view
//   TODO: generate slices and vectors for every layer in the model
//   TODO: write vectors as GCode
// TODO: render textures as slices in IASceneView
// TODO: prototyped - generate slices as OpenGL Textures
// TODO: prototyped - write slices to disk as images
// TODO: prototyped - create vector outline from slices
// TODO: shrink slices
// TODO: create vector infills from slices
// TODO: prototyped - create GCode from vectors
// Done (LOL)

// TODO: port to Linux

#include "Iota.h"

#include "data/binaryData.h"
#include "userinterface/IAGUIMain.h"
#include "fileformats/IAFmtTexJpeg.h"
#include "fileformats/IAFmtObj3ds.h"
#include "fileformats/IAGeometryReader.h"
#include "fileformats/IAGeometryReaderBinaryStl.h"

#include <FL/fl_ask.H>

#include <errno.h>


IAIota Iota;


#define HDR "Error: \"%1$s\"\n\n"

const char *IAIota::kErrorMessage[] =
{
    // NoError
        HDR"No error.",
    // CantOpenFile_STR_BSD
        HDR"Can't open file \"%2$s\":\n%3$s",
    // UnknownFileType_STR
        HDR"Unknown and unsupported file type:\n\"%2$s\"",
    // FileContentCorrupt_STR
        HDR"There seems to be an error inside this file:\n\"%2$s\"",
};


//Fl_RGB_Image *texture = 0L;
//
//IAMeshList gMeshList;
//IASlice gMeshSlice;
//IAPrinter gPrinter; // Allocate default printer
//
//bool gShowSlice = false;
//bool gShowTexture = false;
//FILE *gOutFile;
//
//double minX = 0.0, maxX = 0.0, minY = 0.0, maxY = 0.0, minZ = 0.0, maxZ = 0.0;



IAIota::IAIota()
{
}


IAIota::~IAIota()
{
    delete pMesh;
    if (pErrorString)
        ::free((void*)pErrorString);
}


/**
 * Load a list of files from mass storage.
 *
 * Read geometries, textures, GCode files, or whatever the user throws at us.
 * Files are read in the order in which they appear in the list, and reading
 * stops as soon as one file reader creates an error.
 *
 * This does not delete any previously loaded data.
 *
 * \param list one or more filenames, separated by \n
 * \return nothing, but may sho an error dialog to the user
 */
void IAIota::loadAnyFileList(const char *list)
{
    // FIXME: make sure that old models are not delted

    // loop through all entries in the list
    const char *fnStart = list, *fnEnd = list;
    for (;;) {
        fnEnd = strchr(fnStart, '\n');
        if (!fnEnd) fnEnd = fnStart+strlen(fnStart);
        if (fnEnd!=fnStart) {
            clearError();
            char *filename = (char*)calloc(1, fnEnd-fnStart+1);
            memmove(filename, fnStart, fnEnd-fnStart);
            const char *ext = fl_filename_ext(filename);
            if (strcasecmp(ext, ".stl")==0) {
                Iota.addGeometry(filename);
            } else {
                setError("Load Any File", Error::UnknownFileType_STR, filename);
            }
            if (hadError()) {
                showError();
                break;
            }
        }
        if (*fnEnd==0) break;
        fnStart = fnEnd+1;
    }
    gSceneView->redraw();
}


/**
 Experimental stuff.
 */
void IAIota::menuWriteSlice()
{
    char buf[FL_PATH_MAX];
    sprintf(buf, "%s/slice.jpg", getenv("HOME"));
    gMeshSlice.save(zSlider1->value(), buf);
}

void IAIota::menuQuit()
{
    Iota.gMainWindow->hide();
    Fl::flush();
    exit(0);
}

void IAIota::sliceAll()
{
//    gMeshSlice.generateLidFrom(gMeshList, zSlider1->value());
//    defer slicing until we actually need a to recreate the lid
}


/**
 * Read a geometry from memory.
 */
bool IAIota::addGeometry(const char *name, uint8_t *data, size_t size)
{
    bool ret = false;
    auto reader = IAGeometryReader::findReaderFor(name, data, size);
    if (reader)
        ret = addGeometry(reader);
    return ret;
}


/**
 * Read a geometry from an external file.
 */
bool IAIota::addGeometry(const char *filename)
{
    bool ret = false;
    auto reader = IAGeometryReader::findReaderFor(filename);
    if (reader)
        ret = addGeometry(reader);
    return ret;
}


/**
 * Read a geometry from a Reader.
 */
bool IAIota::addGeometry(std::shared_ptr<IAGeometryReader> reader)
{
    bool ret = false;
    delete Iota.pMesh; Iota.pMesh = nullptr;
    auto geometry = reader->load();
    Iota.pMesh = geometry;
    if (pMesh) {
        pMesh->projectTexture(pMesh->pMax.x()*2, pMesh->pMax.y()*2, IA_PROJECTION_FRONT);
        pMesh->centerOnPrintbed(&gPrinter);
    }
    return ret;
}


/**
 * Clear all information about previous errors.
 *
 * clearError() and showError() are used like brackets in a high level
 * call from the user interface.
 *
 * A typical user interaction starts with clearError(),
 * followed by a function that may create an error,
 * and finalized by showLastError(), which will present the error to the user,
 * only if one occured.
 *
 * \see IAIota::showLastError(), IAIota::hadError()
 */
void IAIota::clearError()
{
    pError = Error::NoError;
    pErrorBSD = 0;
    pErrorString = nullptr;
}


/**
 * Set an error code that may or may not be shown to the user later.
 *
 * Errors most be meaningful to the user first and foremost!
 *
 * \param loc the error location. This description should be useful to the user,
 *        not the programmer
 * \param err the actual error message. Error messages ending in _BSD will store
 *        the 'errno' at the time of this call.
 * \param str error messages ending in _STR require additonal text to print the
 *        error message, usually a file name
 */
void IAIota::setError(const char *loc, Error err, const char *str)
{
    pErrorLocation = loc;
    pError = err;
    pErrorBSD = errno;
    if (pErrorString)
        ::free((void*)pErrorString);
    pErrorString = str ? strdup(str) : nullptr;
}


/**
 * Return true, if setError() was called since the last call to clearError().
 */
bool IAIota::hadError()
{
    return (pError!=Error::NoError);
}


/**
 * Show an alert box with the text of the last error, registered by setError().
 */
void IAIota::showError()
{
    if (hadError()) {
        fl_alert(kErrorMessage[(size_t)pError], pErrorLocation, pErrorString, strerror(pErrorBSD));
    }
}


/**
 * Lunch our app.
 * \todo remember the window position and size in the preferences
 */
int main (int argc, char **argv)
{

    Fl::use_high_res_GL(1);

    Iota.gMainWindow = createIotaAppWindow();
    Iota.gMainWindow->show(argc, argv);
    Fl::flush();

    loadTexture("testcard1024.jpg", defaultTexture);
    Iota.addGeometry("default.stl", defaultModel, sizeof(defaultModel));
    Iota.pMesh->projectTexture(100.0, 100.0, IA_PROJECTION_FRONT);

    gSceneView->redraw();

    return Fl::run();
}
