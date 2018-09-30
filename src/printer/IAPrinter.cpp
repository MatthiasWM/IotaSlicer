//
//  IAPrinter.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAPrinter.h"

#include "Iota.h"
#include "IAPrinterFDM.h"
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


static Fl_Menu_Item layerHeightMenu[] = {
    { "0.1", 0, nullptr, nullptr, 0, 0, 0, 11 },
    { "0.2", 0, nullptr, nullptr, 0, 0, 0, 11 },
    { "0.3", 0, nullptr, nullptr, 0, 0, 0, 11 },
    { "0.4", 0, nullptr, nullptr, 0, 0, 0, 11 },
    { }
};



/**
 Create a default printer.

 For example, an M3D Crane:
    Build Volume: 214 X 214 X 230 mm, bottom front left is 0
    The Crane is designed to extrude at a rate of 5.7 mm^3/s
 Flow Rate (mm^3/s) = Feedrate (mm/s) (Filament Cross-section) (mm^2)*
 Filament Cross-section = pi ((Filament Diameter) / 2)^2 __Filament Cross-section for 1.75mm filament: 2.405 mm^2*

 For Example: You send the command G1 E100 F200, where you extrude 100mm of filament at 200 mm/min. You are using 1.75 mm filament.

 Flow Rate = 8 mm/s = pi (1.75 / 2)^2 (200 / 60) = 2.405 3.333*

 Remember that the G1 move command feedrate parameter, F, uses mm per minute. So divide the feedrate by 60 to obtain the feedrate in mm per second.

 Printing Flow Rate

 When printing, the flow rate depends on your layer height, nozzle diameter and print speed.

 Flow Rate (mm^3/s) = (Extrusion Width)(mm) (Layer Height)(mm) Print Speed (mm/s) Extrusion Width is ~120% of nozzle diameter

 For Example: You have a 0.5 mm nozzle mounted and you are printing at 0.25mm layer height at a print speed of 30 mm/s.

 Extrusion Width = 0.6 mm = 1.2 0.5 Flow Rate = 4.5 mm^3/s = 0.6 0.25 * 30
 */
IAPrinter::IAPrinter(const char *newName)
:   pUUID( strdup(Fl_Preferences::newUUID()) ), // FIXME: this does not work well at all!
    gSlice( this )
{
    setName(newName);
    loadSettings();

    pPrinterSettingList.push_back( new IASettingLabel("uuid", "Printer ID:", uuid()) );
    pPrinterSettingList.push_back( new IASettingText("name", "Printer Name:", pName, 32, "",
                                                     [this]{ ; } ) );
    pPrinterSettingList.push_back( new IASettingLabel("buildVolume", "Build Volume:"));
    // bed shape (Choice, rect, round)
    // printbed width, depth
    pPrinterSettingList.push_back(new IASettingFloat("buildVolume/x", "X:",
                                                     *(pBuildVolume.dataPointer()+0),
                                                     "width in mm",
                                                     [this]{ ; } ) );
    pPrinterSettingList.push_back(new IASettingFloat("buildVolume/y", "Y:",
                                                     *(pBuildVolume.dataPointer()+1),
                                                     "depth in mm",
                                                     [this]{ ; } ) );
    pPrinterSettingList.push_back(new IASettingFloat("buildVolume/z", "Z:",
                                                     *(pBuildVolume.dataPointer()+2),
                                                     "height in mm",
                                                     [this]{ ; } ) );

    // build volume (x, y, z);
    // coordinate zero (front left, back right)
    // zero point offset (x, y, z)
    // # extruders
    //   extruder 1
    //     type (single, changing, mixing)
    //     nozzle diameter
    //     # transports
    //       transport 1
    //


//    pPrinterSettingList.push_back( new IASettingLabel("buildVolume", "Buid Volume:") );
//    pPrinterSettingList.push_back( new IASettingLabel("buildVolume/printable", "Printable Area:") );
//    pPrinterSettingList.push_back( new IASettingFloat("buildVolume/printable/xMin", "Minimal X:", pLayerHeight, "mm",
//                                  [this]{userChangedLayerHeight();} ) );
//    pPrinterSettingList.push_back(
//        new IASettingFloatChoice(
//                                 "diagonal", "Build Volume Diagonal:", pLayerHeight, "mm",
//                                  [this]{userChangedLayerHeight();},
//                                  layerHeightMenu) );
//
    // TODO: add choice of profiles (and how to manage them)

    pSceneSettingList.push_back(
        new IASettingFloatChoice(
            "layerHeight", "Layer Height:", pLayerHeight, "mm",
            [this]{userChangedLayerHeight();},
            layerHeightMenu) );
}


/**
 * Release all resources.
 */
IAPrinter::~IAPrinter()
{
    if (pUUID)
        ::free((void*)pUUID);
    if (pName)
        ::free((void*)pName);
    if (pOutputPath)
        ::free((void*)pOutputPath);
    for (auto &s: pPrinterSettingList)
        delete s;
    for (auto &s: pSceneSettingList)
        delete s;
}


/**
 * Copy properties from another printer.
 */
void IAPrinter::operator=(const IAPrinter &rhs)
{
    pName = strdup(rhs.pName);
    pOutputPath = rhs.pOutputPath ? strdup(rhs.pOutputPath) : nullptr;
    pBuildVolumeMin = rhs.pBuildVolumeMin;
    pBuildVolumeMax = rhs.pBuildVolumeMax;
    pBuildVolume = rhs.pBuildVolume;
    pBuildVolumeRadius = rhs.pBuildVolumeRadius;

    pLayerHeight = rhs.pLayerHeight;
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
void IAPrinter::loadSettings()
{
    if (name()==nullptr || *name()==0) return;

    char buf[FL_PATH_MAX];
    Fl_Preferences prefs(Fl_Preferences::USER, "com.matthiasm.iota.printer", name());
    Fl_Preferences output(prefs, "output");
    output.get("lastFilename", buf, "", sizeof(buf));
    setOutputPath( buf );
}


/**
 * Save all settings of this printer to a Preferences file.
 */
void IAPrinter::saveSettings()
{
    if (name()==nullptr || *name()==0) return;

    Fl_Preferences prefs(Fl_Preferences::USER, "com.matthiasm.iota.printer", name());
    Fl_Preferences output(prefs, "output");
    output.set("lastFilename", outputPath());

    if (pIsPrototype) return;

    const char *path = Iota.gPreferences.printerDefinitionsPath();
    Fl_Preferences printerProperties(path, "Iota Printer Properties", uuid());
    for (auto &s: pPrinterSettingList) {
        s->write(printerProperties);
    }
    printerProperties.set("name", name());
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
    for (auto &s: pSceneSettingList) {
        s->build(treeWidget, IASetting::kSetting);
    }
    treeWidget->end();
}


/**
 * Create the Treeview items for editing the printer properties.
 */
void IAPrinter::buildPrinterSettings(Fl_Tree *treeWidget)
{
    treeWidget->showroot(0);
    treeWidget->item_labelsize(13);
    treeWidget->linespacing(3);
    treeWidget->item_draw_mode(FL_TREE_ITEM_DRAW_DEFAULT);
    treeWidget->clear();
    treeWidget->begin();
    for (auto &s: pPrinterSettingList) {
        s->build(treeWidget, IASetting::kProperty);
    }
    treeWidget->end();
}


/**
 * Change the printer name.
 */
void IAPrinter::setName(const char *name)
{
    if (pName)
        ::free((void*)pName);
    pName = nullptr;
    if (name)
        pName = (char*)::strdup(name);
}


/**
 * Return the name of the printer driver.
 */
const char *IAPrinter::name()
{
    return pName;
}


/**
 * Set the ouput filename.
 */
void IAPrinter::setOutputPath(const char *name)
{
    if (pOutputPath)
        ::free((void*)pOutputPath);
    pOutputPath = nullptr;
    if (name)
        pOutputPath = (char*)::strdup(name);
}


/**
 * Return the filename and path of the output file.
 */
const char *IAPrinter::outputPath()
{
    return pOutputPath;
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
#ifdef __LINUX__
    const char *filename = fl_file_chooser(title, filter, outputPath());
#else
    Fl_Native_File_Chooser fc(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    fc.title(title);
    fc.filter(filter);

    char *path = strdup(outputPath());
    char *s = (char*)fl_filename_name(path);
    if (s) *s = 0;
    fc.directory(path);
    free(path);

    fc.preset_file(fl_filename_name(outputPath()));
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

    if (!filename || !*filename)
        return false;
    fl_filename_absolute(buf, sizeof(buf), filename);
    const char *ext = fl_filename_ext(buf);
    if (!ext || !*ext) {
        fl_filename_setext(buf, sizeof(buf), extension);
    }
    setOutputPath(buf);
    saveSettings();
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
    glTranslated(-pBuildVolumeMin.x(), -pBuildVolumeMin.y(), -pBuildVolumeMin.z());

    // bottom plate
    glColor3ub(200, 200, 200);
    glBegin(GL_POLYGON);
    glVertex3d(pBuildVolumeMin.x(), pBuildVolumeMin.y(), pBuildVolumeMin.z());
    glVertex3d(pBuildVolumeMax.x(), pBuildVolumeMin.y(), pBuildVolumeMin.z());
    glVertex3d(pBuildVolumeMax.x(), pBuildVolumeMax.y(), pBuildVolumeMin.z());
    glVertex3d(pBuildVolumeMin.x(), pBuildVolumeMax.y(), pBuildVolumeMin.z());
    glEnd();

    // bottom grid
    glColor3ub(70, 70, 70);
    int i;
    int xmin = (int)ceil(pBuildVolumeMin.x()/10.0)*10;
    int xmax = (int)floor(pBuildVolumeMax.x()/10.0)*10;
    glBegin(GL_LINES);
    for (i=xmin; i<=xmax; i+=10) {
        if (i==0) { glEnd(); glLineWidth(2.5); glBegin(GL_LINES); }
        glVertex3d(i, pBuildVolumeMin.y(), pBuildVolumeMin.z());
        glVertex3d(i, pBuildVolumeMax.y(), pBuildVolumeMin.z());
        if (i==0) { glEnd(); glLineWidth(1.0); glBegin(GL_LINES); }
    }
    glEnd();
    int ymin = (int)ceil(pBuildVolumeMin.y()/10.0)*10;
    int ymax = (int)floor(pBuildVolumeMax.y()/10.0)*10;
    glBegin(GL_LINES);
    for (i=ymin; i<=ymax; i+=10) {
        if (i==0) { glEnd(); glLineWidth(2.5); glBegin(GL_LINES); }
        glVertex3d(pBuildVolumeMin.x(), i, pBuildVolumeMin.z());
        glVertex3d(pBuildVolumeMax.x(), i, pBuildVolumeMin.z());
        if (i==0) { glEnd(); glLineWidth(1.0); glBegin(GL_LINES); }
    }
    glEnd();

    glColor3ub(70, 70, 70);

    // top
    glBegin(GL_LINE_LOOP);
    glVertex3d(pBuildVolumeMin.x(), pBuildVolumeMin.y(), pBuildVolumeMax.z());
    glVertex3d(pBuildVolumeMax.x(), pBuildVolumeMin.y(), pBuildVolumeMax.z());
    glVertex3d(pBuildVolumeMax.x(), pBuildVolumeMax.y(), pBuildVolumeMax.z());
    glVertex3d(pBuildVolumeMin.x(), pBuildVolumeMax.y(), pBuildVolumeMax.z());
    glEnd();

    // corners
    glBegin(GL_LINES);
    glVertex3d(pBuildVolumeMin.x(), pBuildVolumeMin.y(), pBuildVolumeMin.z());
    glVertex3d(pBuildVolumeMin.x(), pBuildVolumeMin.y(), pBuildVolumeMax.z());
    glVertex3d(pBuildVolumeMax.x(), pBuildVolumeMin.y(), pBuildVolumeMin.z());
    glVertex3d(pBuildVolumeMax.x(), pBuildVolumeMin.y(), pBuildVolumeMax.z());
    glVertex3d(pBuildVolumeMax.x(), pBuildVolumeMax.y(), pBuildVolumeMin.z());
    glVertex3d(pBuildVolumeMax.x(), pBuildVolumeMax.y(), pBuildVolumeMax.z());
    glVertex3d(pBuildVolumeMin.x(), pBuildVolumeMax.y(), pBuildVolumeMin.z());
    glVertex3d(pBuildVolumeMin.x(), pBuildVolumeMax.y(), pBuildVolumeMax.z());
    glEnd();

    // bottom frame
    glBegin(GL_LINE_LOOP);
    glVertex3d(pBuildVolumeMin.x(), pBuildVolumeMin.y(), pBuildVolumeMin.z());
    glVertex3d(pBuildVolumeMax.x(), pBuildVolumeMin.y(), pBuildVolumeMin.z());
    glVertex3d(pBuildVolumeMax.x(), pBuildVolumeMax.y(), pBuildVolumeMin.z());
    glVertex3d(pBuildVolumeMin.x(), pBuildVolumeMax.y(), pBuildVolumeMin.z());
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
    printf("New layer height is %f\n", pLayerHeight);
}


