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
#include "userinterface/IAGUIMain.h"

#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Window.H>


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
//    Iota.pPrinterList.add(new IAPrinterFDMBelt("Generic FDM Belt Printer"));
    Iota.pPrinterList.add(new IAPrinterInkjet("Generic Inkjet Printer"));
    Iota.pPrinterList.add(new IAPrinterLasercutter("Generic Laser Cutter"));
//    Iota.pPrinterList.add(new IAPrinterSLS("Generic SLS Printer"));
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
        IAPrinter *ret = new IAPrinterFDM("default printer");
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
    for (auto &p: pPrinterList) {
        m->label(p->name());
        m->callback((Fl_Callback*)userSelectsPrinterCB, p);
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
void IAPrinterList::userSelectsPrinter(IAPrinter *p)
{
    Iota.pCurrentPrinter = p;
    Iota.gMainWindow->redraw();
    wPrinterName->copy_label(p->name());
    buildSessionSettings(p);
}


void IAPrinterList::buildSessionSettings(IAPrinter *p)
{
    wSessionSettings->showroot(0);
    wSessionSettings->item_labelsize(11);
//    wSessionSettings->spacing
    wSessionSettings->linespacing(5);
    wSessionSettings->item_draw_mode(FL_TREE_ITEM_DRAW_LABEL_AND_WIDGET);
    wSessionSettings->clear();
    wSessionSettings->begin();
    p->buildSessionSettings();
    wSessionSettings->end();
}


/**
 * User selected a printer from the menu item list.
 */
void IAPrinterList::userSelectsPrinterCB(Fl_Menu_Item*, void *p)
{
    Iota.pPrinterList.userSelectsPrinter((IAPrinter*)p);
}


