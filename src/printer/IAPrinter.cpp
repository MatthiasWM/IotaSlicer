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
    setUUID(src.uuid());
    setName(src.name());
    setOutputPath(src.outputPath());
    pBuildVolumeMin = src.pBuildVolumeMin;
    pBuildVolumeMax = src.pBuildVolumeMax;
    pBuildVolume = src.pBuildVolume;
    pBuildVolumeRadius = src.pBuildVolumeRadius;
    pLayerHeight = src.pLayerHeight;
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
    for (auto &s: pPrinterProperties)
        delete s;
    for (auto &s: pSceneSettings)
        delete s;
}


void IAPrinter::initializePrinterProperties()
{
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
    pPrinterProperties.push_back( new IASettingLabel("uuid", "Printer ID:", uuid()) );
    pPrinterProperties.push_back( new IASettingText("name", "Printer Name:", pName, 32, "",
                                                     [this]{ ; } ) );
    pPrinterProperties.push_back( new IASettingLabel("buildVolume", "Build Volume:"));
    // bed shape (Choice, rect, round)
    // printbed width, depth
    pPrinterProperties.push_back(new IASettingFloat("buildVolume/x", "X:",
                                                     *(pBuildVolume.dataPointer()+0),
                                                     "width in mm",
                                                     [this]{ ; } ) );
    pPrinterProperties.push_back(new IASettingFloat("buildVolume/y", "Y:",
                                                     *(pBuildVolume.dataPointer()+1),
                                                     "depth in mm",
                                                     [this]{ ; } ) );
    pPrinterProperties.push_back(new IASettingFloat("buildVolume/z", "Z:",
                                                     *(pBuildVolume.dataPointer()+2),
                                                     "height in mm",
                                                     [this]{ ; } ) );
}


void IAPrinter::initializeSceneSettings()
{
    // TODO: add choice of profiles (and how to manage them)

    static Fl_Menu_Item layerHeightMenu[] = {
        { "0.1", 0, nullptr, nullptr, 0, 0, 0, 11 },
        { "0.2", 0, nullptr, nullptr, 0, 0, 0, 11 },
        { "0.3", 0, nullptr, nullptr, 0, 0, 0, 11 },
        { "0.4", 0, nullptr, nullptr, 0, 0, 0, 11 },
        { }
    };

    IASetting *s;
    s = new IASettingFloatChoice("layerHeight", "Layer Height:", pLayerHeight, "mm",
                                 [this]{userChangedLayerHeight();},
                                 layerHeightMenu);
    pSceneSettings.push_back(s);
}



/**
 * Copy properties from another printer.
 */
//void IAPrinter::operator=(const IAPrinter &rhs)
//{
//    pName = strdup(rhs.pName);
//    pOutputPath = rhs.pOutputPath ? strdup(rhs.pOutputPath) : nullptr;
//    pBuildVolumeMin = rhs.pBuildVolumeMin;
//    pBuildVolumeMax = rhs.pBuildVolumeMax;
//    pBuildVolume = rhs.pBuildVolume;
//    pBuildVolumeRadius = rhs.pBuildVolumeRadius;
//
//    pLayerHeight = rhs.pLayerHeight;
//}


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
//    char buf[FL_PATH_MAX];
//    Fl_Preferences prefs(Fl_Preferences::USER, "com.matthiasm.iota.printer", name());
//    Fl_Preferences output(prefs, "output");
//    output.get("lastFilename", buf, "", sizeof(buf));
//    setOutputPath( buf );

    const char *path = Iota.gPreferences.printerDefinitionsPath();
    Fl_Preferences printerProperties(path, "Iota Printer Properties", uuid());
    for (auto &s: pPrinterProperties) {
        s->read(printerProperties);
    }
}


/**
 * Save all settings of this printer to a Preferences file.
 */
void IAPrinter::saveSettings()
{
//    Fl_Preferences prefs(Fl_Preferences::USER, "com.matthiasm.iota.printer", name());
//    Fl_Preferences output(prefs, "output");
//    output.set("lastFilename", outputPath());

    const char *path = Iota.gPreferences.printerDefinitionsPath();
    Fl_Preferences printerProperties(path, "Iota Printer Properties", uuid());
    for (auto &s: pPrinterProperties) {
        s->write(printerProperties);
    }
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
    for (auto &s: pPrinterProperties) {
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
const char *IAPrinter::name() const
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
const char *IAPrinter::outputPath() const
{
    return pOutputPath;
}


/**
 * Generate a new UUID.
 */
void IAPrinter::setNewUUID()
{
    setUUID( Fl_Preferences::newUUID() );
}


/**
 * Set a new UUID.
 */
void IAPrinter::setUUID(const char *uuid)
{
    if (pUUID)
        ::free((void*)pUUID);
    pUUID = nullptr;
    if (uuid)
        pUUID = (char*)::strdup(uuid);
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


