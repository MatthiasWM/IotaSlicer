//
//  IAPrinter.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "controller/IAPrinterListController.h"

#include "Iota.h"
#include "printer/IAPrinter.h"
#include "printer/IAPrinterFDM.h"
#include "printer/IAPrinterFDMBelt.h"
#include "printer/IAPrinterInkjet.h"
#include "printer/IAPrinterLasercutter.h"
#include "printer/IAPrinterSLS.h"
#include "view/IAGUIMain.h"

#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Window.H>


IAPrinterListController::IAPrinterListController()
{
}


IAPrinterListController::~IAPrinterListController()
{
}


/**
 * This is called if the currently edited printer name changed.
 */
void IAPrinterListController::preferencesNameChanged()
{
    if (!wSettingsPrinterList) return;

    // find the printer that is edited using the printer list
    int ix = wSettingsPrinterList->value();
    if (ix==0) return;

    IAPrinter *printer = (IAPrinter*)wSettingsPrinterList->data(ix);
    assert(printer);

    // get the name of the printer
    const char *name = printer->name();

    // update the printer list widget
    wSettingsPrinterList->text(ix, name);
    wSettingsPrinterList->redraw();

    // update the printer choice widegt
    if (wPrinterChoice && printer==Iota.pCurrentPrinter) {
        Fl_Menu_Item *m = (Fl_Menu_Item*)wPrinterChoice->mvalue();
        m->label(name);
        wPrinterChoice->redraw();
    }
}


void IAPrinterListController::preferencesPrinterSeleted()
{
    if (!wSettingsPrinterList) return;

    // find the printer that is edited using the printer list
    IAPrinter *p = nullptr;
    int line = wSettingsPrinterList->value();
    Iota.pUserDialogSettingsSelectedPrinterIndex = line;
    if (line) p = (IAPrinter*)wSettingsPrinterList->data(line);
    if (p) {
        // update the properties tree widget
        p->buildPropertiesUI(wSettingsPrinterProperties);
    } else {
        // or clear the properties tree widget
        wSettingsPrinterProperties->clear();
    }
    wSettingsPrinterProperties->redraw();
}


void IAPrinterListController::preferencesPrinterDeseleted()
{
    if (!wSettingsPrinterList) return;

    // find the printer that was previously edited
    IAPrinter *p = nullptr;
    int line = Iota.pUserDialogSettingsSelectedPrinterIndex;
    if (line) p = (IAPrinter*)wSettingsPrinterList->data(line);
    if (p) {
        // deselecting a printer will save its properties
        p->saveProperties();
    }
    wSettingsPrinterProperties->clear();
}





