//
//  IAController.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_CONTROLLER_H
#define IA_CONTROLLER_H


#include "Iota.h"

#include <vector>
#include <functional>

#include <FL/Fl_Preferences.H>

class IACtrlTreeItemFloat;
class IALabelView;
class IAFloatView;
class IATextView;
class IAFloatProperty;
class IAFloatChoiceView;
class IAIntProperty;
class IAChoiceView;

struct Fl_Menu_Item;
class Fl_Input_Choice;
class Fl_Choice;
class Fl_Tree;
class Fl_Tree_Item;
class Fl_Preferences;
struct Fl_Menu_Item;


class IAController
{
public:
    IAController();
    virtual ~IAController();
    virtual void propertyValueChanged();
    void autoDelete(bool v);
    bool isAutoDelete() { return pAutoDelete; }
private:
    bool pAutoDelete = false;
};


class IACallbackController : public IAController
{
public:
    IACallbackController(IAProperty &prop, std::function<void()>&& cb);
    virtual ~IACallbackController();
    virtual void propertyValueChanged() override;
    IAProperty &pProperty;
    std::function<void()> pCallback;
};


// FIXME: what actually happens whe the tree is cleared? Tree-Items deleted? Widgets stay in Group? ???
class IATreeItemController : public IAController
{
public:
    typedef enum { kProperty, kSetting } Type;
    IATreeItemController(const char *path, const char *label);
    virtual ~IATreeItemController() override;
    virtual void build(Fl_Tree*, Type, int) { }
    void tooltip(const char *text) { pTooltip = text; }

    Fl_Menu_Item *dup(Fl_Menu_Item const*);

    char *pPath = nullptr;
    char *pLabel = nullptr;
    const char *pTooltip = nullptr;
    Fl_Tree_Item *pTreeItem = nullptr;
};



/**
 * Manage a setting that appears in a tree view.
 */
class IALabelController : public IATreeItemController
{
public:
    IALabelController(const char *path, const char *label, const char *text=nullptr);
    virtual ~IALabelController() override;
    virtual void build(Fl_Tree*, Type, int) override;

    char *pText = nullptr;
    IALabelView *pWidget = nullptr;
};



/**
 * Manage a setting that appears in a tree view.
 */
class IAFloatController : public IATreeItemController
{
public:
    IAFloatController(const char *path, const char *label, IAFloatProperty &prop,
                      const char *unit, std::function<void()>&& cb);
    virtual ~IAFloatController() override;
    virtual void build(Fl_Tree*, Type, int) override;
    virtual void propertyValueChanged() override;

    static void wCallback(IAFloatView *w, IAFloatController *d);

    IAFloatProperty &pProperty;
    char *pUnit = nullptr;
    std::function<void()> pCallback;
    IAFloatView *pWidget = nullptr;
};



/**
 * Manage a setting that appears in a tree view.
 */
class IATextController : public IATreeItemController
{
public:
    IATextController(const char *path, const char *label, IATextProperty &prop, int wdt,
                     const char *unit, std::function<void()>&& cb);
    virtual ~IATextController() override;
    virtual void build(Fl_Tree*, Type, int) override;
    virtual void propertyValueChanged() override;

    static void wCallback(IATextView *w, IATextController *d);

    IATextProperty &pProperty;
    int pWdt;
    char *pUnit = nullptr;
    std::function<void()> pCallback;
    IATextView *pWidget = nullptr;
};




/**
 * Manage a setting that appears in a tree view.
 */
class IAFloatChoiceController : public IATreeItemController
{
public:
    IAFloatChoiceController(const char *path, const char *label, IAFloatProperty &prop,
                            const char *unit, std::function<void()>&& cb,
                            Fl_Menu_Item *menu);
    virtual ~IAFloatChoiceController() override;
    virtual void build(Fl_Tree*, Type, int) override;
    virtual void propertyValueChanged() override;

    static void wCallback(IAFloatChoiceView *w, IAFloatChoiceController *d);

    IAFloatProperty &pProperty;
    char *pUnit = nullptr;
    std::function<void()> pCallback;
    Fl_Menu_Item *pMenu = nullptr;
    IAFloatChoiceView *pWidget = nullptr;
};


/**
 * Manage a setting that appears in a tree view.
 */
class IAChoiceController : public IATreeItemController
{
public:
    IAChoiceController(const char *path, const char *label, IAIntProperty &prop,
                       std::function<void()>&& cb, Fl_Menu_Item *menu);
    virtual ~IAChoiceController() override;
    virtual void build(Fl_Tree*, Type, int) override;
    virtual void propertyValueChanged() override;

    static void wCallback(IAChoiceView *w, IAChoiceController *d);

    IAIntProperty &pProperty;
    Fl_Menu_Item *pMenu = nullptr;
    std::function<void()> pCallback;
    IAChoiceView *pWidget = nullptr;
};


class IAVectorController : public IATreeItemController
{
public:
    IAVectorController(const char *path, const char *label, const char *text,
                       IAVectorProperty &prop,
                       char const* xLabel, char const* xUnits,
                       char const* yLabel, char const* yUnits,
                       char const* zLabel, char const* zUnits,
                       std::function<void()>&& cb);
    virtual ~IAVectorController() override;
    virtual void build(Fl_Tree*, Type, int) override;
    virtual void propertyValueChanged() override;

    static void wCallback(IAFloatView *w, IAVectorController *d);

    IAVectorProperty &pProperty;
    std::function<void()> pCallback;
    IALabelView *pLabelWidget = nullptr;
    IAFloatView *pXWidget = nullptr;
    IAFloatView *pYWidget = nullptr;
    IAFloatView *pZWidget = nullptr;
    char* pText = nullptr;
    char* pXPath = nullptr;
    char* pYPath = nullptr;
    char* pZPath = nullptr;
    char const* pXLabel = nullptr;
    char const* pXUnits = nullptr;
    char const* pYLabel = nullptr;
    char const* pYUnits = nullptr;
    char const* pZLabel = nullptr;
    char const* pZUnits = nullptr;
};


typedef std::vector<IATreeItemController*> IAControllerList;


#endif /* IA_PRINTER_LIST_CONTROLLER_H */


