//
//  Iota.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "Iota.h"

#include "data/binaryData.h"
#include "view/IAGUIMain.h"
#include "app/IAVersioneer.h"
#include "fileformats/IAFmtTexJpeg.h"
#include "fileformats/IAFmtObj3ds.h"
#include "fileformats/IAGeometryReader.h"
#include "fileformats/IAGeometryReaderBinaryStl.h"
#include "opengl/IAFramebuffer.h"
#include "toolpath/IAToolpath.h"
#include "printer/IAPrinter.h"


#include <FL/fl_ask.H>
#include <FL/fl_utf8.h>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Tooltip.H>

#include <errno.h>

#ifdef _WIN32
#include <ShlObj.h>
#endif


IAIota Iota;

IAVersioneer gVersioneer;

// Composition
// 35/18mm, 10/34/34mm, 3x12mm


/* Do not change the somewhat funky [ver stuff below. It is used for automated
 * version number updates.
 */
const char *gVersion = /*[ver*/"v0.3.2b"/*]*/;

//const int kFramebufferSize = 2048;
//const int kFramebufferSize = 4096;
const int kFramebufferSize = 214*20;

#ifdef __APPLE__
#pragma mark -
#endif
// ==== Iota ===================================================================


/**
 * Create the Iota Slicer application.
 */
IAIota::IAIota()
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
    // fc.directory(gPreferences.pLastLoadFilename); pRecentFile[]...
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
    userMenuFileNewProject();
    loadAnyFile(filename);
}


/**
 * Load a previously loaded file again.
 */
void IAIota::userMenuOpenRecentFile(int ix)
{
    const char *filename = gPreferences.pRecentFile[ix];
    if (filename && *filename) {
        loadAnyFile(filename);
    }
}


/**
 * User wants to clear the list of recntly loaded files.
 */
void IAIota::userMenuClearRecentFileList()
{
    gPreferences.clearRecentFileList();
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


/**
 * User wants to see the printer editor in the settings panel.
 */
void IAIota::userMenuSettingsManagePrinter()
{
    if (!wSettingsWindow) {
        wSettingsWindow = createSettingWindow();
    }
    IAPrinter *p = pCurrentPrinter;
    pCustomPrinterList.fillBrowserWidget(wSettingsPrinterList, p);
    wSettingsPrinterList->do_callback();

    /** \todo choose printer teb */
    wSettingsWindow->show();
}


// ---- menu Help --------------------------------------------------------------


/**
 * Open the version managment tool (not for the user mode release!).
 */
void IAIota::userMenuHelpVersioneer()
{
    gVersioneer.show();
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


// ---- main window ------------------------------------------------------------


void IAIota::userMainSelectPrinter()
{
    Fl_Menu_Item const* m = wPrinterChoice->mvalue();
    if (m) {
        Iota.pCurrentPrinter = (IAPrinter*)m->user_data();
        Iota.pCurrentPrinter->buildSessionSettings(wSessionSettings);
    }
}


#ifdef __APPLE__
#pragma mark -
#endif
// =============================================================================


/**
 * User clicked into the printer list and selected a printer to edit.
 *
 * This fills the the preferences view with the properties of the newly
 * selected printer.
 */
void IAIota::userDialogSettingPrinterSelect()
{
    pPrinterListController.preferencesPrinterSeleted();
}


/**
 * User interaction that ends editing the last selected printer.
 *
 * This saves the current printer settings/
 */
void IAIota::userDialogSettingPrinterDeselect()
{
    pPrinterListController.preferencesPrinterDeseleted();
}


/**
 * User clicked the [+] button in the Printer panel of the setting dialog.
 *
 * This pops up a menu that lets the user choose to duplicate the currently
 * selected printer or duplicate any of the default printer types.
 *
 * \todo implement printer [+] callback
 */
void IAIota::userDialogSettingPrinterAdd()
{
    char buf[FL_PATH_MAX];
    Fl_Menu_Item *menu = pPrinterPrototypeList.createPrinterAddMenu();
    IAPrinter *p = nullptr;
    int line = wSettingsPrinterList->value();
    pUserDialogSettingsSelectedPrinterIndex = line;
    if (line) p = (IAPrinter*)wSettingsPrinterList->data(line);
    if (p) {
        snprintf(buf, sizeof(buf), "Duplicate \"%s\"", p->name());
        menu[0].label(buf);
        menu[0].activate();
        menu[0].user_data(p);
    } else {
        strcpy(buf, "Duplicate selected");
        menu[0].label(buf);
        menu[0].deactivate();
    }
    const Fl_Menu_Item *m = menu->popup(wSettingPrinterAdd->x(), wSettingPrinterAdd->y()+wSettingPrinterAdd->h());
    if (m) {
        IAPrinter *p = (IAPrinter*)m->user_data();
        if (p) {
            pCustomPrinterList.cloneAndAdd(p);
            pCustomPrinterList.fillBrowserWidget(wSettingsPrinterList, p);
            pCustomPrinterList.updatePrinterSelectMenu();
        }
    }
    ::free((void*)menu);
}


/**
 * User clicked the [-] button in the Printer panel of the setting dialog.
 *
 * If a printer is selected, this pops up a warning dialog to confirm the
 * deletion of a printer, and deletes the printer if confirmed.
 */
void IAIota::userDialogSettingPrinterRemove()
{
    if (pCustomPrinterList.size()<=1) return;
    IAPrinter *p = nullptr;
    int line = wSettingsPrinterList->value();
    if (line) p = (IAPrinter*)wSettingsPrinterList->data(line);
    if (p) {
        int ret = fl_choice("This will permanently delete all setting\n"
                            "for the printer named\n\"%s\"",
                            nullptr, "Cancel Operation", "Delete Printer",
                            p->name());
        if (ret==2) {
            pCustomPrinterList.remove(p);
            pCustomPrinterList.fillBrowserWidget(wSettingsPrinterList, 0);
            pCustomPrinterList.updatePrinterSelectMenu();
            pUserDialogSettingsSelectedPrinterIndex = 0;
            p->deletePropertiesFile();
            pCustomPrinterList.saveCustomPrinters();
            wSettingsPrinterList->do_callback();
            if (pCurrentPrinter==p) {
                wPrinterChoice->value(0);
                wPrinterChoice->do_callback();
            }
            delete p; p = nullptr;
        }
    }
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
            if (Error.hadError()) {
                Error.showDialog();
                ::free((void*)filename);
                break;
            } else {
                gPreferences.addRecentFile(filename);
                ::free((void*)filename);
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
    Fl_Tooltip::size(12);

    Iota.pPrinterPrototypeList.generatePrototypes();
    Iota.pCustomPrinterList.loadCustomPrinters(Iota.pCurrentPrinter);

    Iota.gMainWindow = createIotaAppWindow();
    {
        char buf[80];
        snprintf(buf, 79, "Iota Voxel Slicer %s", gVersion);
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
    Iota.pCustomPrinterList.updatePrinterSelectMenu();
    int currentPrinter = Iota.gPreferences.pCurrentPrinterIndex;
    if (currentPrinter>=Iota.pCustomPrinterList.size())
        currentPrinter = 0;
    wPrinterChoice->value(currentPrinter);
    Iota.pCurrentPrinter = Iota.pCustomPrinterList[currentPrinter];
    Iota.pCurrentPrinter->buildSessionSettings(wSessionSettings);
    Iota.gMainWindow->show();

    Iota.loadDemoFiles();
    gSceneView->redraw();

    return Fl::run();
}
