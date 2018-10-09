//
//  IAPrinterListController.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_PRINTER_LIST_CONTROLLER_H
#define IA_PRINTER_LIST_CONTROLLER_H


#include <vector>


class IAPrinter;
struct Fl_Menu_Item;
class Fl_Browser;


/**
 * Manage a list of printers.
 */
class IAPrinterListController
{
public:
    IAPrinterListController();
    ~IAPrinterListController();

    void preferencesNameChanged();
    void preferencesPrinterSeleted();
    void preferencesPrinterDeseleted();
};


#endif /* IA_PRINTER_LIST_CONTROLLER_H */


