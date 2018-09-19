//
//  Iota.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "Iota.h"

#include "data/binaryData.h"
#include "userinterface/IAGUIMain.h"
#include "fileformats/IAFmtTexJpeg.h"
#include "fileformats/IAFmtObj3ds.h"
#include "fileformats/IAGeometryReader.h"
#include "fileformats/IAGeometryReaderBinaryStl.h"
#include "opengl/IAFramebuffer.h"
#include "toolpath/IAToolpath.h"

#include <FL/fl_ask.H>
#include <FL/fl_utf8.h>
#include <FL/Fl_Native_File_Chooser.H>

#include <errno.h>

#ifdef _WIN32
#include <ShlObj.h>
#endif


IAIota Iota;

/* Do not change the somewhat funky [ver stuff below. It is used for automated
 * version number updates.
 */
const char *gVersion = /*[ver*/"v0.0.10a"/*]*/;

const int kFramebufferSize = 2048;
//const int kFramebufferSize = 4096;


#ifdef __APPLE__
#pragma mark -
#endif
// ==== Iota ===================================================================


/**
 * Creat the Iota Slicer application.
 */
IAIota::IAIota()
:   pCurrentPrinter( nullptr ),
    pPrinterList( wPrinterSelectionMenu )
{
}


/**
 * Release all resources.
 * \todo add a clear() function, so we can reset Iota into accepting a new scene.
 */
IAIota::~IAIota()
{
    delete pMesh;
}


#ifdef __APPLE__
#pragma mark -
#endif
// ==== handle the main menu ===================================================


// ---- menu File --------------------------------------------------------------


/**
 * Clear the build playform and start a new build.
 */
void IAIota::userMenuFileNewProject()
{
    delete Iota.pMesh; Iota.pMesh = nullptr;
    if (pCurrentPrinter)
        pCurrentPrinter->purgeSlicesAndCaches();
    gSceneView->redraw();
}


/**
 * Load any kind of file from disk.
 *
 * Currently that would be STL files
 */
void IAIota::userMenuFileOpen()
{
    Fl_Native_File_Chooser fc(Fl_Native_File_Chooser::BROWSE_FILE);
    fc.title("Open mesh file");
    fc.filter("*.{stl,STL}");
    fc.directory(gPreferences.pLastLoadFilename);
    switch (fc.show()) {
        case -1: // error
        case 1: // cancel
            return;
        default: // filename choosen
            break;
    }
    const char *filename = fc.filename();
    if (!filename || !*filename)
        return;
    fl_filename_absolute(gPreferences.pLastLoadFilename,
                         sizeof(gPreferences.pLastLoadFilename),
                         filename);
    gPreferences.flush();
    userMenuFileNewProject();
    addGeometry(gPreferences.pLastLoadFilename);
}


/**
 * Just quit the app.
 * \todo At some point, we should probably ask if the user has unsaved changes.
 */
void IAIota::userMenuFileQuit()
{
    Iota.gMainWindow->hide();
    Fl::flush();
    exit(0);
}


// ---- menu Edit --------------------------------------------------------------

// nothing yet

// ---- menu View --------------------------------------------------------------

// nothing yet

// ---- menu Slice -------------------------------------------------------------


/**
 * Sve the sliced object (GCode, DXF, images) to disk or SDCard
 *
 * The first time in a session, the printer driver verifies the destination
 * filename by calling userSaveAs(). For further calls, this will repeat
 * the previous operation for a given printer.
 */
void IAIota::userMenuSliceSave()
{
    if (pCurrentPrinter)
        pCurrentPrinter->userSliceSave();
}


/**
 * Ask for a filename and write the slice data there.
 */
void IAIota::userMenuSliceSaveAs()
{
    if (pCurrentPrinter)
        pCurrentPrinter->userSliceSaveAs();
}


/**
 * Clean all preprocessed and cached slice data.
 *
 * Iota should be smart about cleaning caches whenever the user changes relevant
 * settings. This menu item is somewhat of a reassurance and also a helper
 * during development.
 */
void IAIota::userMenuSliceClean()
{
    if (pCurrentPrinter)
        pCurrentPrinter->purgeSlicesAndCaches();
    zRangeSlider->lowValue( zRangeSlider->highValue() );
    gSceneView->redraw();
}


/**
 * User wants to slice the entire scene.
 */
void IAIota::userMenuSliceSliceAll()
{
    if (pCurrentPrinter)
        pCurrentPrinter->userSliceGenerateAll();
}


// ---- menu Settings ----------------------------------------------------------

// nothing yet

// ---- menu Help --------------------------------------------------------------


/**
 * Open the version managment tool (not for the user mode release!).
 */
void IAIota::userMenuHelpVersioneer()
{
    if (!wVersionWindow) {
        createVersionWindow();
    }
    loadSettings();
    wVersionWindow->show();
}


/**
 * Open the "About..." Iota window.
 */
void IAIota::userMenuHelpAbout()
{
    static Fl_Window *aboutWindow = nullptr;
    if (!aboutWindow)
        aboutWindow = createIotaAboutWindow();
    aboutWindow->show();
}


#ifdef __APPLE__
#pragma mark -
#endif
// =============================================================================



/**
 * Load a list of files from mass storage.
 *
 * Read geometries, textures, GCode files, or whatever the user throws at us.
 * Files are read in the order in which they appear in the list, and reading
 * stops as soon as one file reader creates an error.
 *
 * This does not delete any previously loaded data. Use clear() for that.
 *
 * This does not return an error code. Query succes by calling hadError()
 * and possibly showError().
 *
 * \param list one or more filenames, separated by \n
 *
 * \todo At this point, we only know how to read STL files.
 * \todo Currently, we only support one mesh, which will be replaced by
 *       whatever we read.
 */
void IAIota::loadAnyFile(const char *list)
{
    // loop through all entries in the list
    const char *fnStart = list, *fnEnd = list;
    for (;;) {
        fnEnd = strchr(fnStart, '\n');
        if (!fnEnd) fnEnd = fnStart+strlen(fnStart);
        if (fnEnd!=fnStart) {
            // We found a filename; now go and read that file.
            Error.clear();
            char *filename = (char*)calloc(1, fnEnd-fnStart+1);
            memmove(filename, fnStart, fnEnd-fnStart);
            const char *ext = fl_filename_ext(filename);
            if (fl_utf_strcasecmp(ext, ".stl")==0) {
                Iota.addGeometry(filename);
            } else {
                Error.set("Load Any File", IAError::UnknownFileType_STR, filename);
            }
            ::free((void*)filename);
            if (Error.hadError()) {
                Error.showDialog();
                break;
            }
        }
        if (*fnEnd==0) break;
        fnStart = fnEnd+1;
    }
    gSceneView->redraw();
}


/**
 * Read a geometry from memory.
 *
 * \param name is needed to define the file type by its extension (test.stl, etc.)
 * \param data binary clone of the file in memory
 * \param size of data in bytes
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
 *
 * \param filename the extension of the file name is used to help determine the file type
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
 * Read a geometry from any Reader.
 *
 * \todo Make sure that all buffered data is invalidated when adding another
 *       mesh to the scene.
 *
 * \param read a file reader previously generated by other addGeometry calls
 */
bool IAIota::addGeometry(std::shared_ptr<IAGeometryReader> reader)
{
    bool ret = false;
    delete Iota.pMesh; Iota.pMesh = nullptr;
    if (pCurrentPrinter)
        pCurrentPrinter->purgeSlicesAndCaches();
    auto geometry = reader->load();
    Iota.pMesh = geometry;
    if (pMesh) {
        pMesh->projectTexture(pMesh->pMax.x()*2, pMesh->pMax.y()*2, IA_PROJECTION_FRONT);
        pMesh->projectTexture(3, 1, IA_PROJECTION_CYLINDRICAL);
        pMesh->centerOnPrintbed(pCurrentPrinter);
    }
    return ret;
}


/**
 * Load a model and a texture that come with the app for an easy example.
 *
 * These are binary assets compiled into the code. There is no need for
 * external files.
 */
void IAIota::loadDemoFiles()
{
    loadTexture("testcard1024.jpg", defaultTexture);
    Iota.addGeometry("default.stl", defaultModel, sizeof(defaultModel));
}


#ifdef __APPLE__
#pragma mark -
#endif
// ==== error handling =========================================================


#define HDR "Error in Iota Slicer, %s\n\n"


/// user definable string explaining the details of an error
const char *IAError::pErrorString = nullptr;

/// user definable string explaining the function that caused an error
const char *IAError::pErrorLocation = nullptr;

/// current error condition
IAError::Error IAError::pError = IAError::NoError;

/// system specific error number of the call that was markes by an error
int IAError::pErrorBSD = 0;

/**
 * Error messages for error code from the Error enum class.
 *
 * These strings are called with the varargs
 *  1) function: a human readable location where this error occured.
 *  2) a caller provided string, often a file name
 *  3) the cause of an error as a text returned by the operating system
 */
const char *IAError::kErrorMessage[] =
{
    // NoError
    HDR"No error.",
    // CantOpenFile_STR_BSD
    HDR"Can't open file \"%s\":\n%s",
    // UnknownFileType_STR
    HDR"Unknown and unsupported file type:\n\"%s\"",
    // FileContentCorrupt_STR
    HDR"There seems to be an error inside this file:\n\"%s\"",
    // OpenGLFeatureNotSupported_STR
    HDR"Required OpenGL graphics feature not suported:\n\"%s\"",
};


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
void IAError::clear()
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
void IAError::set(const char *loc, Error err, const char *str)
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
 *
 * \return true if there was an error since the last call to clearError()
 */
bool IAError::hadError()
{
    return (pError!=Error::NoError);
}


/**
 * Show an alert box with the text of the last error, registered by setError().
 */
void IAError::showDialog()
{
    if (hadError()) {
        fl_alert(kErrorMessage[(size_t)pError], pErrorLocation, pErrorString, strerror(pErrorBSD));
    }
}



#ifdef __APPLE__
#pragma mark -
#endif
// ==== main() =================================================================


/**
 * Launch our app.
 *
 * \todo The whole user interface must be in its own class.
 */
int main (int argc, char **argv)
{

    Fl::scheme("gtk+");
	Fl::args(argc, argv);
    Fl::set_color(FL_BACKGROUND_COLOR, 0xeeeeee00);
    Fl::use_high_res_GL(1);

    Iota.pCurrentPrinter = Iota.pPrinterList.defaultPrinter();

    Iota.gMainWindow = createIotaAppWindow();
    {
        char buf[80];
        sprintf(buf, "Iota Voxe Slicer %s", gVersion);
        Iota.gMainWindow->copy_label(buf);
    }
    if (Iota.gPreferences.pMainWindowX==-1) {
        Iota.gMainWindow->size(Iota.gPreferences.pMainWindowW,
                               Iota.gPreferences.pMainWindowH);
    } else {
        Iota.gMainWindow->resize(Iota.gPreferences.pMainWindowX,
                                 Iota.gPreferences.pMainWindowY,
                                 Iota.gPreferences.pMainWindowW,
                                 Iota.gPreferences.pMainWindowH);
    }
    Iota.gMainWindow->show();
    Fl::flush();

    Iota.pPrinterList.userSelectsPrinter(Iota.pPrinterList.defaultPrinter());
    Iota.loadDemoFiles();

    gSceneView->redraw();

    return Fl::run();
}
