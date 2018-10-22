//
//  IAPrinter.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAPrinter.h"

#include "Iota.h"
#include "IAFDMPrinter.h"
#include "IAPrinterFDMBelt.h"
#include "IAPrinterInkjet.h"
#include "IAPrinterLasercutter.h"
#include "IAPrinterSLS.h"
#include "view/IAGUIMain.h"
#include "toolpath/IAToolpath.h"

#include <math.h>

#include <FL/gl.h>
#include <FL/glu.h>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Window.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/filename.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Tree_Item.H>



/**
 Create a default printer.

 For example, an M3D Crane:
    Build Volume: 214 X 214 X 230 mm, bottom front left is 0
    The Crane is designed to extrude at a rate of 5.7 mm^3/s
 */
IAPrinter::IAPrinter()
{
}


IAPrinter::IAPrinter(IAPrinter const& src)
:   IAPrinter()
{
    pPrintVolume = src.pPrintVolume;
    pPrintVolumeRadius = src.pPrintVolumeRadius;

    uuid.set( src.uuid() );
    name.set( src.name() );
    presetClass.set( src.presetClass() );
    recentUpload.set( src.recentUpload() );
    motionRangeMin.set( src.motionRangeMin() );
    motionRangeMax.set( src.motionRangeMax() );
    printVolumeMin.set( src.printVolumeMin() );
    printVolumeMax.set( src.printVolumeMax() );
    layerHeight.set( src.layerHeight() );
}


/**
 * Release all resources.
 */
IAPrinter::~IAPrinter()
{
    for (auto &s: pPropertiesControllerList)
        delete s;
    for (auto &s: pSceneSettings)
        delete s;
}


void IAPrinter::createPropertiesControls()
{
    IATreeItemController *s;

    // -- display the UUID (this is not really important for the user)
    s = new IALabelController("uuid", "Printer ID:", uuid());
    s->tooltip("This unique ID is used for debugging purposes. Please ignore.");
    pPropertiesControllerList.push_back(s);

    // -- editable name of this printer
    s = new IATextController("name", "Printer Name:", name, 32, "",
                             []{ Iota.pPrinterListController.preferencesNameChanged(); } );
    s->tooltip("Give this printer description a human readable name.");
    pPropertiesControllerList.push_back(s);

    // -- group presets of similar printers in a class
    s = new IATextController("presetClass", "Preset Class:", presetClass, 32, "",
                             []{ /* reloadPresets(); */ } );
    s->tooltip("Group presets of similar printers in a class.");
    pPropertiesControllerList.push_back(s);

    // -- recentUpload is handled by the "upload" menu.

    // -- motionRange and printVolume controlles should be added by the derived printer classes.
}


void IAPrinter::initializeSceneSettings()
{
    /// \todo add choice of profiles (and how to manage them)

    static Fl_Menu_Item layerHeightMenu[] = {
        { "0.1", 0, nullptr, nullptr, 0, 0, 0, 11 },
        { "0.2", 0, nullptr, nullptr, 0, 0, 0, 11 },
        { "0.3", 0, nullptr, nullptr, 0, 0, 0, 11 },
        { "0.4", 0, nullptr, nullptr, 0, 0, 0, 11 },
        { }
    };

    IATreeItemController *s;
    s = new IAFloatChoiceController("layerHeight", "Layer Height:", layerHeight, "mm",
                              [this]{userChangedLayerHeight();},
                              layerHeightMenu);
    pSceneSettings.push_back(s);
}


/**
 * Clear all buffered data and prepare for a modified scene.
 */
void IAPrinter::purgeSlicesAndCaches()
{
    gSlice.clear();
    gSceneView->redraw();
}


/**
 * Load all settings of this printer to a Preferences file.
 *
 * The file path is the general app data area for every platform, followed by
 * the vendor string. The filename is the name of the printer.
 */
void IAPrinter::readPropertiesFile()
{
    const char *path = Iota.gPreferences.printerDefinitionsPath();
    Fl_Preferences printer(path, "Iota Printer Properties", uuid());
    readProperties(printer);
}


void IAPrinter::readProperties(Fl_Preferences &printer)
{
    uuid.read(printer);
    name.read(printer);
    presetClass.read(printer);
    recentUpload.read(printer);

    Fl_Preferences properties(printer, "properties");
    motionRangeMin.read(properties);
    motionRangeMax.read(properties);
    printVolumeMin.read(properties);
    printVolumeMax.read(properties);
}


/**
 * Save all settings of this printer to a Preferences file.
 */
void IAPrinter::writePropertiesFile()
{
    const char *path = Iota.gPreferences.printerDefinitionsPath();
    Fl_Preferences printer(path, "Iota Printer Properties", uuid());
    writeProperties(printer);
}


void IAPrinter::writeProperties(Fl_Preferences &printer)
{
    uuid.write(printer);
    name.write(printer);
    presetClass.write(printer);
    recentUpload.write(printer);

    Fl_Preferences properties(printer, "properties");
    motionRangeMin.write(properties);
    motionRangeMax.write(properties);
    printVolumeMin.write(properties);
    printVolumeMax.write(properties);
}


void IAPrinter::deletePropertiesFile()
{
    char buf[FL_PATH_MAX];
    strcpy(buf, Iota.gPreferences.printerDefinitionsPath());
    strcat(buf, uuid());
    strcat(buf, ".prefs");
    fl_unlink(buf);
}


/**
 * Create the Treeview items for setting up the printout for this session.
 */
void IAPrinter::buildSessionSettings(Fl_Tree *treeWidget)
{
    treeWidget->showroot(0);
    treeWidget->item_labelsize(13);
    treeWidget->linespacing(3);
    treeWidget->item_draw_mode(FL_TREE_ITEM_DRAW_DEFAULT);
    treeWidget->clear();
    treeWidget->begin();
    wSessionSettings->begin();
    for (auto &s: pSceneSettings) {
        s->build(treeWidget, IATreeItemController::kSetting, treeWidget->w()-40);
    }
    treeWidget->end();
}


/**
 * Create the Treeview items for editing the printer properties.
 */
void IAPrinter::createPropertiesViews(Fl_Tree *treeWidget)
{
    treeWidget->showroot(0);
    treeWidget->item_labelsize(13);
    treeWidget->linespacing(3);
    treeWidget->item_draw_mode(FL_TREE_ITEM_DRAW_DEFAULT);
    treeWidget->clear();
    treeWidget->begin();
    for (auto &s: pPropertiesControllerList) {
        s->build(treeWidget, IATreeItemController::kProperty, treeWidget->w()-100);
    }
    treeWidget->end();
}


/**
 * Generate a new UUID.
 */
void IAPrinter::setNewUUID()
{
    uuid.set( Fl_Preferences::newUUID() );
}


/**
 * Ask the user to give us a filename for writing.
 *
 * \param title a free text that is displayed at the top of the dialog
 * \param filter highlight certain filename, for example "*.{jpg|png}"
 * \param extension add this extension if none is given
 *
 * \return false, if the user canceled the request
 */
bool IAPrinter::queryOutputFilename(const char *title,
                                  const char *filter,
                                  const char *extension)
{
    char buf[FL_PATH_MAX];

    char *path = nullptr;
    if (recentUpload()) {
        path = strdup(recentUpload());
    } else {
        const char *home = fl_getenv("HOME");
        if (home) {
            strcpy(buf, home);
            strcat(buf, "/");
        } else {
            strcpy(buf, "./");
        }
        path = strdup(buf);
    }

#ifdef __LINUX__
    const char *filename = fl_file_chooser(title, filter, path);
#else
    Fl_Native_File_Chooser fc(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    fc.title(title);
    fc.filter(filter);

    char *s = (char*)fl_filename_name(path);
    if (s) s[-1] = 0;
    fc.directory(path);

    if (s) fc.preset_file(s); else fc.preset_file("");
    fc.options(Fl_Native_File_Chooser::NEW_FOLDER |
               Fl_Native_File_Chooser::PREVIEW);
    switch (fc.show()) {
        case -1: // error
        case 1: // cancel
            return false;
        default: // filename choosen
            break;
    }
    const char *filename = fc.filename();
#endif

    free(path);

    if (!filename || !*filename)
        return false;
    fl_filename_absolute(buf, sizeof(buf), filename);
    const char *ext = fl_filename_ext(buf);
    if (!ext || !*ext) {
        fl_filename_setext(buf, sizeof(buf), extension);
    }
    recentUpload.set(buf);
    writePropertiesFile();
    pFirstWrite = false;
    return true;
}


/**
 * Draw a minimal shape representing the printer in the scene.
 */
void IAPrinter::draw()
{
    // draw printing volume using OpenGL
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    // draw the printer so that the bottom left front corner of the build
    // platform is in the OpenGL origin
    glPushMatrix();
    glTranslated(-printVolumeMin().x(), -printVolumeMin().y(), -printVolumeMin().z());

    // bottom plate
    glColor3ub(200, 200, 200);
    glBegin(GL_POLYGON);
    glVertex3d(printVolumeMin().x(), printVolumeMin().y(), printVolumeMin().z());
    glVertex3d(printVolumeMax().x(), printVolumeMin().y(), printVolumeMin().z());
    glVertex3d(printVolumeMax().x(), printVolumeMax().y(), printVolumeMin().z());
    glVertex3d(printVolumeMin().x(), printVolumeMax().y(), printVolumeMin().z());
    glEnd();

    // bottom grid
    glColor3ub(70, 70, 70);
    int i;
    int xmin = (int)ceil(printVolumeMin().x()/10.0)*10;
    int xmax = (int)floor(printVolumeMax().x()/10.0)*10;
    glBegin(GL_LINES);
    for (i=xmin; i<=xmax; i+=10) {
        if (i==0) { glEnd(); glLineWidth(2.5); glBegin(GL_LINES); }
        glVertex3d(i, printVolumeMin().y(), printVolumeMin().z());
        glVertex3d(i, printVolumeMax().y(), printVolumeMin().z());
        if (i==0) { glEnd(); glLineWidth(1.0); glBegin(GL_LINES); }
    }
    glEnd();
    int ymin = (int)ceil(printVolumeMin().y()/10.0)*10;
    int ymax = (int)floor(printVolumeMax().y()/10.0)*10;
    glBegin(GL_LINES);
    for (i=ymin; i<=ymax; i+=10) {
        if (i==0) { glEnd(); glLineWidth(2.5); glBegin(GL_LINES); }
        glVertex3d(printVolumeMin().x(), i, printVolumeMin().z());
        glVertex3d(printVolumeMax().x(), i, printVolumeMin().z());
        if (i==0) { glEnd(); glLineWidth(1.0); glBegin(GL_LINES); }
    }
    glEnd();

    glColor3ub(70, 70, 70);

    // top
    glBegin(GL_LINE_LOOP);
    glVertex3d(printVolumeMin().x(), printVolumeMin().y(), printVolumeMax().z());
    glVertex3d(printVolumeMax().x(), printVolumeMin().y(), printVolumeMax().z());
    glVertex3d(printVolumeMax().x(), printVolumeMax().y(), printVolumeMax().z());
    glVertex3d(printVolumeMin().x(), printVolumeMax().y(), printVolumeMax().z());
    glEnd();

    // corners
    glBegin(GL_LINES);
    glVertex3d(printVolumeMin().x(), printVolumeMin().y(), printVolumeMin().z());
    glVertex3d(printVolumeMin().x(), printVolumeMin().y(), printVolumeMax().z());
    glVertex3d(printVolumeMax().x(), printVolumeMin().y(), printVolumeMin().z());
    glVertex3d(printVolumeMax().x(), printVolumeMin().y(), printVolumeMax().z());
    glVertex3d(printVolumeMax().x(), printVolumeMax().y(), printVolumeMin().z());
    glVertex3d(printVolumeMax().x(), printVolumeMax().y(), printVolumeMax().z());
    glVertex3d(printVolumeMin().x(), printVolumeMax().y(), printVolumeMin().z());
    glVertex3d(printVolumeMin().x(), printVolumeMax().y(), printVolumeMax().z());
    glEnd();

    // bottom frame
    glBegin(GL_LINE_LOOP);
    glVertex3d(printVolumeMin().x(), printVolumeMin().y(), printVolumeMin().z());
    glVertex3d(printVolumeMax().x(), printVolumeMin().y(), printVolumeMin().z());
    glVertex3d(printVolumeMax().x(), printVolumeMax().y(), printVolumeMin().z());
    glVertex3d(printVolumeMin().x(), printVolumeMax().y(), printVolumeMin().z());
    glEnd();

    // origin and coordinate system
    glLineWidth(2.5);
    glBegin(GL_LINES);
    glColor3ub(255, 0, 0);
    glVertex3d(0, 0, 0);
    glVertex3d(50, 0, 0);
    glColor3ub(0, 255, 0);
    glVertex3d(0, 0, 0);
    glVertex3d(0, 50, 0);
    glColor3ub(0, 0, 255);
    glVertex3d(0, 0, 0);
    glVertex3d(0, 0, 50);
    glEnd();

    glLineWidth(1.0);
    glPopMatrix();
}


/**
 * Draw a preview of the slicing operation.
 */
void IAPrinter::drawPreview(double lo, double hi)
{
}


void IAPrinter::userChangedLayerHeight()
{
    printf("New layer height is %f\n", layerHeight());
}


void IAPrinter::updateBuildVolume()
{
    printVolumeMin().z( 0.0 );
    printVolumeMax().z( motionRangeMax().z() );
    pPrintVolume = printVolumeMax() - printVolumeMin();
    pPrintVolumeRadius = pPrintVolume.length()*2.5;
}





