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
#include "userinterface/IAGUIMain.h"

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
{
    setName(newName);
    loadSettings();
}


/**
 * Release all resources.
 */
IAPrinter::~IAPrinter()
{
    if (pName)
        ::free((void*)pName);
    if (pOutputPath)
        ::free((void*)pOutputPath);
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
}


/**
 * Change the printer name and save the new settings.
 */
void IAPrinter::setName(const char *name)
{
    if (pName)
        ::free((void*)pName);
    pName = nullptr;
    if (name)
        pName = (char*)::strdup(name);
    saveSettings();
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
 *
 * The filename will be saved for each named printer and restored when Iota
 * is started again.
 */
void IAPrinter::setOutputPath(const char *name)
{
    if (pOutputPath)
        ::free((void*)pOutputPath);
    pOutputPath = nullptr;
    if (name)
        pOutputPath = (char*)::strdup(name);
    saveSettings();
}


/**
 * Return the filename and path of the output file.
 */
const char *IAPrinter::outputPath()
{
    return pOutputPath;
}


/**
 * User asked us to select a file or directory, slice all layers in the scene,
 * and save the result.
 */
void IAPrinter::userSliceAs()
{
    fl_message("The printer\n\"%s\"\ndoes not support slicing yet.", name());
}


/**
 * User asks us to slice and save to the previously used output file.
 *
 * If this is the first request in this session, make sure that the user
 * still agrees with the previous filename.
 */
void IAPrinter::userSliceAgain()
{
    if (pFirstWrite) {
        userSliceAs();
    } else {
        sliceAndWrite();
    }
}


/**
 * Slice the entire scene and write the result to disk.
 */
void IAPrinter::sliceAndWrite(const char *filename)
{
    /* empty */
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

    Fl_Native_File_Chooser fc(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    fc.title(title);
    fc.filter(filter);
    fc.directory(outputPath());
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
    if (!filename || !*filename)
        return false;
    //    strcpy(gPreferences.pLastGCodeFilename, filename);
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


//===========================================================================//


/**
 * Manage a number of printers.
 *
 * Not setting printermenu should manage a printer list without user interface.
 *
 * \param printermenu a menu item that will be modified to point at all the
 *        printers in this list.
 */
IAPrinterList::IAPrinterList(Fl_Menu_Item *printermenu)
:   pMenuItem( printermenu )
{
    Iota.pPrinterList.add(new IAPrinterFDM("Generic FDM Printer"));
    Iota.pPrinterList.add(new IAPrinterFDMBelt("Generic FDM Belt Printer"));
    Iota.pPrinterList.add(new IAPrinterInkjet("Generic Inkjet Printer"));
    Iota.pPrinterList.add(new IAPrinterLasercutter("Generic Laser Cutter"));
    Iota.pPrinterList.add(new IAPrinterSLS("Generic SLS Printer"));
}


/**
 * Relase all userinterface parts of the list.
 *
 * Does not release the individual printers.
 */
IAPrinterList::~IAPrinterList()
{
    if (pMenuItem) {
        pMenuItem->flags &= ~FL_SUBMENU_POINTER;
        pMenuItem->user_data(nullptr);
    }
    if (pMenuArray) {
        ::free((void*)pMenuArray);
    }
}


/**
 * Return the first printer in the list, or a new generic printer, if there
 * is none.
 */
IAPrinter *IAPrinterList::defaultPrinter()
{
    if (pPrinterList.size()) {
        return pPrinterList[0];
    } else {
        IAPrinter *ret = new IAPrinter("default printer");
        // automatically adds this printer to the list
        return ret;
    }
}


/**
 * Add a printer to the list and rebuild the menu item list.
 */
bool IAPrinterList::add(IAPrinter *printer)
{
    pPrinterList.push_back(printer);
    buildMenuArray();
    return true;
}


/**
 * Buold a menu array and link it to the printer selection menu.
 *
 * \todo a checkmark at the selected printer would be nice
 * \todo just show customized printers, not the generic ones
 * \todo append menu item "Add new printer..."
 * \todo create a printer creation dialog
 */
void IAPrinterList::buildMenuArray()
{
    if (pMenuArray) ::free((void*)pMenuArray);

    pMenuArray = (Fl_Menu_Item*)::calloc( sizeof(Fl_Menu_Item),
                                         pPrinterList.size()+1);
    Fl_Menu_Item *m = pMenuArray;
    for (auto p: pPrinterList) {
        m->label(p->name());
        m->callback((Fl_Callback*)userSelectedPrinterCB, p);
        m++;
    }

    pMenuItem->flags |= FL_SUBMENU_POINTER;
    pMenuItem->user_data(pMenuArray);
}


/**
 * Select another printer.
 *
 * \todo flush all kinds of things
 * \todo move this code into Iota class
 * \todo redraw the entire user interface
 */
void IAPrinterList::userSelectedPrinter(IAPrinter *p)
{
    Iota.pCurrentPrinter = p;
    Iota.gMainWindow->redraw();
    wPrinterName->copy_label(p->name());
//    Fl_Tree_Item *it;
    wSessionSettings->showroot(0);
    wSessionSettings->item_labelsize(12);
    wSessionSettings->clear();
    wSessionSettings->add("Quality");
    wSessionSettings->add("Quality/Resolution (pulldown)");
    wSessionSettings->add("Quality/Color (pulldown)");
    wSessionSettings->add("Quality/Details");
    wSessionSettings->add("Quality/Details/Layer Height");
    wSessionSettings->add("Quality/Details/...");
    wSessionSettings->add("Hotend 1");
    wSessionSettings->add("Hotend 1/Filament 1 (pulldown)");
    wSessionSettings->add("Hotend 2");
    wSessionSettings->add("Hotend 2/Filament 2 (pulldown)");
    wSessionSettings->add("Scene");
    wSessionSettings->add("Scene/Colormode (pulldown)");
}


/**
 * User selected a printer from the menu item list.
 */
void IAPrinterList::userSelectedPrinterCB(Fl_Menu_Item*, void *p)
{
    Iota.pPrinterList.userSelectedPrinter((IAPrinter*)p);
}




