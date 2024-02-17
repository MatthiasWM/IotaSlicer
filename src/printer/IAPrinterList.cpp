//
//  IAPrinter.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "printer/IAPrinterList.h"

#include "Iota.h"
#include "printer/IAFDMPrinter.h"
#include "printer/IAPrinterFDMBelt.h"
#include "printer/IAPrinterInkjet.h"
#include "printer/IAPrinterLasercutter.h"
#include "printer/IAPrinterSLS.h"
#include "view/IAGUIMain.h"

#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Window.H>

#include <algorithm>


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
 *
 * By not assigning a UUID, it is clear that these printers are prototypes.
 */
void IAPrinterList::generatePrototypes()
{
    IAPrinter *p;

    // add an FDM printer prototype
    p = new IAFDMPrinter();
    p->name.set("Generic FDM Printer");
    add(p);

    // add an inkjet printer prototype
    p = new IAPrinterInkjet();
    p->name.set("Generic Inkjet Printer");
    add(p);

    // add a lasercutter printer prototype
    p = new IAPrinterLasercutter();
    p->name.set("Generic Laser Cutter");
    add(p);
}


/**
 * Read all user created printers from a preference file.
 *
 * Create at least on generic printer if no user generated printers were found.
 */
void IAPrinterList::loadCustomPrinters(IAPrinter *(&currentPrinter))
{
    const char *path = Iota.gPreferences.printerDefinitionsPath();
    do {
        Fl_Preferences customPrinterList(path, "Iota Printer List", "customPrinters", Fl_Preferences::C_LOCALE);
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
                printer = new IAFDMPrinter();
            } else if (strcmp(type, "IAPrinterInkjet")==0) {
                printer = new IAPrinterInkjet();
            } else if (strcmp(type, "IAPrinterLasercutter")==0) {
                printer = new IAPrinterLasercutter();
            }
            if (printer) {
                printer->uuid.set( uuid );
                printer->createPropertiesControls();
                printer->initializeSceneSettings();
                printer->readPropertiesFile();
                add(printer);
            }
        }
    } while(0);
    if (pPrinterList.size()==0) {
        cloneAndAdd(Iota.pPrinterPrototypeList.pPrinterList[0]);
    }
    currentPrinter = pPrinterList[0];
}


IAPrinter *IAPrinterList::cloneAndAdd(IAPrinter *p)
{
    IAPrinter *printer = p->clone();
    printer->setNewUUID();
    char *newName = makeUniqueName(printer->name());
    printer->name.set(newName);
    printer->createPropertiesControls();
    printer->initializeSceneSettings();
    ::free((void*)newName);
    add(printer);
    saveCustomPrinters();
    printer->writePropertiesFile();
    return printer;
}


/**
 * Modify the given name to make it unique in the printer list.
 *
 * \return a string that must be free'd.
 */
char *IAPrinterList::makeUniqueName(char const* name) const
{
    // copy or create a valid name
    char buf[FL_PATH_MAX];
    if (name && name[0]) {
        strcpy(buf, name);
    } else {
        strcpy(buf, "new Printer");
    }
    // does that name exist already?
    bool dup = false;
    for (auto &p: pPrinterList) {
        if (strcmp(p->name(), buf)==0) { dup = true; break; }
    }
    // it's unique! Return it.
    if (!dup) return strdup(buf);
    // try other names by using a number as an extension
    fl_filename_setext(buf, sizeof(buf), ".%d");
    char buf2[FL_PATH_MAX];
    int i, n = (int)pPrinterList.size()+2;
    for (i=1; i<n; ++i) {
        snprintf(buf2, sizeof(buf2), buf, i);
        dup = false;
        for (auto &p: pPrinterList) {
            if (strcmp(p->name(), buf2)==0) { dup = true; break; }
        }
        // it's unique! Return it.
        if (!dup) return strdup(buf2);
    }
    return strdup("unnamed");
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
    Fl_Preferences customPrinterList(path, "Iota Printer List", "customPrinters", Fl_Preferences::C_LOCALE);
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
 * Add a printer to the list.
 */
bool IAPrinterList::add(IAPrinter *printer)
{
    pPrinterList.push_back(printer);
    return true;
}


/**
 * Remove a printer from the list.
 */
void IAPrinterList::remove(IAPrinter *printer)
{
    auto it = std::find(pPrinterList.begin(), pPrinterList.end(), printer);
    if (it!=pPrinterList.end()) {
        pPrinterList.erase( it );
    }
}


/**
 * Create a menuitem list that allows the user to duplicate printers in
 * this list.
 *
 * \return a list of meu items that must be free'd by the caller.
 */
Fl_Menu_Item *IAPrinterList::createPrinterAddMenu()
{
    int n = (int)pPrinterList.size();
    Fl_Menu_Item *mi = (Fl_Menu_Item*)calloc(n+3, sizeof(Fl_Menu_Item));
    mi[0].label("Duplicate selected");
    mi[0].flags |= FL_MENU_DIVIDER;
    mi[0].labelsize(12);
    mi[1].label("Duplicate Prototype:");
    mi[1].flags |= FL_MENU_INACTIVE;
    mi[1].labelsize(12);
    for (int i=0; i<n; i++) {
        IAPrinter *p = pPrinterList[i];
        mi[i+2].label(p->name());
        mi[i+2].user_data(p);
        mi[i+2].labelsize(12);
    }
    return mi;
}


/**
 * Create a menuitem list that allows the used to select a printer from
 * this list.
 *
 * \return a list of meu items that must be free'd by the caller.
 */
Fl_Menu_Item *IAPrinterList::createPrinterSelectMenu()
{
    int n = (int)pPrinterList.size();
    Fl_Menu_Item *mi = (Fl_Menu_Item*)calloc(n+1, sizeof(Fl_Menu_Item));
    for (int i=0; i<n; i++) {
        IAPrinter *p = pPrinterList[i];
        mi[i].label(p->name());
        mi[i].user_data(p);
        mi[i].labelsize(12);
    }
    return mi;
}


void IAPrinterList::updatePrinterSelectMenu()
{
    if (wPrinterChoice) {
        if (wPrinterChoice->menu()) {
            ::free((void*)wPrinterChoice->menu());
        }
        Fl_Menu_Item *m = createPrinterSelectMenu();
        wPrinterChoice->menu(m);
    }
}


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


