//
//  IAPrinter.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAPrinterList.h"

#include "Iota.h"
#include "IAPrinterFDM.h"
#include "IAPrinterFDMBelt.h"
#include "IAPrinterInkjet.h"
#include "IAPrinterLasercutter.h"
#include "IAPrinterSLS.h"
#include "view/IAGUIMain.h"

#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Window.H>


/**
 * Manage a number of printers.
 *
 * This list holds customized printers or printer prototypes. Customized
 * printers are based on other customized printers or on printer prototypes.
 * Printer prototypes can not be edited or removed.
 *
 * New custom printers are added by selecting an existing printer and
 * duplicating it. Existing printers can be deleted in the printer setting
 * editor.
 */
IAPrinterList::IAPrinterList()
{
}


/**
 * Relase all view parts of the list.
 *
 * Does not release the individual printers.
 */
IAPrinterList::~IAPrinterList()
{
    for (auto &p: pPrinterList)
        delete p;
//    if (pMenuItem) {
//        pMenuItem->flags &= ~FL_SUBMENU_POINTER;
//        pMenuItem->user_data(nullptr);
//    }
//    if (pMenuArray) {
//        ::free((void*)pMenuArray);
//    }
}


/**
 * Create an entry for every supported type of printer.
 */
void IAPrinterList::generatePrototypes()
{
    add(new IAPrinterFDM("Generic FDM Printer"));
//  add(new IAPrinterFDMBelt("Generic FDM Belt Printer"));
    add(new IAPrinterInkjet("Generic Inkjet Printer"));
    add(new IAPrinterLasercutter("Generic Laser Cutter"));
//  add(new IAPrinterSLS("Generic SLS Printer"));
}


/**
 * Read all user created printers from a preference file.
 *
 * Create at least on generic printer if no user generated printers were found.
 */
void IAPrinterList::loadCustomPrinters(IAPrinter *(&currentPrinter))
{
    const char *path = Iota.gPreferences.printerDefinitionsPath();
    Fl_Preferences customPrinterList(path, "Iota Printer List", "customPrinters");
    Fl_Preferences printers(customPrinterList, "Printers");
    int i, n = printers.groups();
    for (i=0; i<n; i++) {
        Fl_Preferences printerRef(printers, printers.group(i));
        char uuid[128], name[128], type[128];
        printerRef.get("uuid", uuid, "", 128);
        printerRef.get("name", name, "", 128);
        printerRef.get("type", type, "", 128);
        IAPrinter *printer = nullptr;
        if (strcmp(type, "IAPrinterFDM")==0) {
            printer = new IAPrinterFDM(name);
        } else if (strcmp(type, "IAPrinterInkjet")==0) {
            printer = new IAPrinterInkjet(name);
        } else if (strcmp(type, "IAPrinterLasercutter")==0) {
            printer = new IAPrinterLasercutter(name);
        }
        if (printer) {
            // FIXME: read printer configuration from preferences file
            add(printer);
        }
    }
    if (pPrinterList.size()==0) {
        add(Iota.pPrinterPrototypeList.pPrinterList[0]->clone());
        // FIXME: saveCustomPrinters();
    }
    currentPrinter = pPrinterList[0];
}


/**
 * Write the list of user created printers to a preference file.
 *
 * Create at least on generic printer if no user generated printers were found.
 *
 * \todo Do we want to write the currently selected printer here?
 */
void IAPrinterList::saveCustomPrinters()
{
    const char *path = Iota.gPreferences.printerDefinitionsPath();
    Fl_Preferences customPrinterList(path, "Iota Printer List", "customPrinters");
    Fl_Preferences printers(customPrinterList, "Printers");
    printers.clear();
    int i, n = (int)pPrinterList.size();
    for (i=0; i<n; i++) {
        Fl_Preferences printerRef(printers, Fl_Preferences::Name(i));
        IAPrinter *printer = pPrinterList[i];
        printerRef.set("uuid", printer->uuid());
        printerRef.set("name", printer->name());
        printerRef.set("type", printer->type());
    }
}


/**
 * Return the first printer in the list, or a new generic printer, if there
 * is none.
 */
//IAPrinter *IAPrinterList::defaultPrinter()
//{
//    if (pPrinterList.size()) {
//        return pPrinterList[0];
//    } else {
//        IAPrinter *ret = new IAPrinterFDM("default printer");
//        // automatically adds this printer to the list
//        return ret;
//    }
//}


/**
 * Add a printer to the list and rebuild the menu item list.
 */
bool IAPrinterList::add(IAPrinter *printer)
{
    pPrinterList.push_back(printer);
//    buildMenuArray();
    return true;
}


/**
 * Create a menuitem list that allows the used to duplicate printers in
 * this list.
 *
 * \return a list of meu items that must be free'd by the caller.
 */
Fl_Menu_Item *IAPrinterList::createPrinterAddMenu()
{
    int n = (int)pPrinterList.size();
    Fl_Menu_Item *mi = (Fl_Menu_Item*)calloc(n+3, sizeof(Fl_Menu_Item));
    mi[0].label("Duplicate selected"); // FIXME: put the name of the selected printer here!
    mi[0].flags |= FL_MENU_DIVIDER;
    mi[1].label("Duplicate Prototype:");
    mi[1].flags |= FL_MENU_INACTIVE;
    for (int i=0; i<n; i++) {
        IAPrinter *p = pPrinterList[i];
        mi[i+2].label(p->name());
        mi[i+2].user_data(p);
    }
    return mi;
}


/**
 * Buold a menu array and link it to the printer selection menu.
 *
 * \todo a checkmark at the selected printer would be nice
 * \todo just show customized printers, not the generic ones
 * \todo append menu item "Add new printer..."
 * \todo create a printer creation dialog
 */
//void IAPrinterList::buildMenuArray()
//{
//    if (pMenuArray) ::free((void*)pMenuArray);
//
//    pMenuArray = (Fl_Menu_Item*)::calloc( sizeof(Fl_Menu_Item),
//                                         pPrinterList.size()+1);
//    Fl_Menu_Item *m = pMenuArray;
//    for (auto &p: pPrinterList) {
//        m->label(p->name());
//        m->callback((Fl_Callback*)userSelectsPrinterCB, p);
//        m++;
//    }
//
//    pMenuItem->flags |= FL_SUBMENU_POINTER;
//    pMenuItem->user_data(pMenuArray);
//}


/**
 * Select another printer.
 *
 * \todo flush all kinds of things
 * \todo move this code into Iota class
 * \todo redraw the entire user interface
 */
//void IAPrinterList::userSelectsPrinter(IAPrinter *p)
//{
//    Iota.pCurrentPrinter = p;
//    Iota.gMainWindow->redraw();
//    wPrinterName->copy_label(p->name());
//    buildSessionSettings(p);
//}


//void IAPrinterList::buildSessionSettings(IAPrinter *p)
//{
//    wSessionSettings->showroot(0);
//    wSessionSettings->item_labelsize(13);
//    wSessionSettings->linespacing(3);
//    wSessionSettings->item_draw_mode(FL_TREE_ITEM_DRAW_DEFAULT);
//    wSessionSettings->clear();
//    wSessionSettings->begin();
//    p->buildSceneSettings(wSessionSettings);
//    wSessionSettings->end();
//}


/**
 * User selected a printer from the menu item list.
 */
//void IAPrinterList::userSelectsPrinterCB(Fl_Menu_Item*, void *p)
//{
//    Iota.pPrinterPrototypeList.userSelectsPrinter((IAPrinter*)p);
//}


void IAPrinterList::fillBrowserWidget(Fl_Browser *browser, IAPrinter *select)
{
    browser->clear();
    int ix = 1, ixSelected = 0;
    for (auto &p: pPrinterList) {
        browser->add(p->name(), p);
        if (p==select) ixSelected = ix;
        ix++;
    }
    if (ixSelected) browser->value(ixSelected);
}


