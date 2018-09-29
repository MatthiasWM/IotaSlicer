//
//  IAPrinterList.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_PRINTER_LIST_H
#define IA_PRINTER_LIST_H


#include <vector>


class IAPrinter;
struct Fl_Menu_Item;


/**
 * Manage a list of printers.
 */
class IAPrinterList
{
public:
    IAPrinterList();
    ~IAPrinterList();
    bool add(IAPrinter *printer);
//    IAPrinter *defaultPrinter();
//    void userSelectsPrinter(IAPrinter *p);

    void generatePrototypes();
    void loadCustomPrinters(IAPrinter *(&p));

    Fl_Menu_Item *createPrinterAddMenu();

private:
//    void buildMenuArray();
//    void buildSessionSettings(IAPrinter *p);
//    static void userSelectsPrinterCB(Fl_Menu_Item*, void *p);

//    Fl_Menu_Item *pMenuItem = nullptr;
//    Fl_Menu_Item *pMenuArray = nullptr;

    std::vector<IAPrinter *> pPrinterList;
};


#endif /* IA_PRINTER_LIST_H */


