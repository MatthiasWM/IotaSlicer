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


/**
 * Controllers connect Properties to one or more Views.
 *
 * A controller manages communication between a view and a property. One
 * controller connects one view to a property. A property can have any number
 * of controllers, and hence any number of views. When a property changes,
 * all controllers are signaled, and all views are updated.
 *
 * \see IAProperty, IAView
 */
class IAController
{
public:
    IAController();
    virtual ~IAController();
    virtual void propertyValueChanged(IAProperty*);
    void autoDelete(bool v);
    bool isAutoDelete() { return pAutoDelete; }
private:
    bool pAutoDelete = false;
};


/**
 * This controller calls a custom function when a property changes.
 *
 * This controller has no view associtaed to itself.
 */
class IACallbackController : public IAController
{
public:
    IACallbackController(IAProperty &prop, std::function<void()>&& cb);
    virtual ~IACallbackController();
    virtual void propertyValueChanged(IAProperty*) override;
    IAProperty &pProperty;
    std::function<void()> pCallback;
};


/**
 * This is the controller base class for view that are constructed in an Fl_Tree.
 *
 * \todo what actually happens whe the tree is cleared? Tree-Items deleted? Widgets stay in Group? ???
 */
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
 * Manage a label and an additional text that appears in a tree view.
 *
 * This Controller is static and not associted to a property.
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
 * Manage a floating point property that appears in a tree view as a text input field.
 */
class IAFloatController : public IATreeItemController
{
public:
    IAFloatController(const char *path, const char *label, IAFloatProperty &prop,
                      const char *unit, std::function<void()>&& cb);
    virtual ~IAFloatController() override;
    virtual void build(Fl_Tree*, Type, int) override;
    virtual void propertyValueChanged(IAProperty*) override;

    static void wCallback(IAFloatView *w, IAFloatController *d);

    IAFloatProperty &pProperty;
    char *pUnit = nullptr;
    std::function<void()> pCallback;
    IAFloatView *pWidget = nullptr;
};


/**
 * Manage a text property that appears in a tree view as a text input field.
 */
class IATextController : public IATreeItemController
{
public:
    IATextController(const char *path, const char *label, IATextProperty &prop, int wdt,
                     const char *unit, std::function<void()>&& cb);
    virtual ~IATextController() override;
    virtual void build(Fl_Tree*, Type, int) override;
    virtual void propertyValueChanged(IAProperty*) override;

    static void wCallback(IATextView *w, IATextController *d);

    IATextProperty &pProperty;
    int pWdt;
    char *pUnit = nullptr;
    std::function<void()> pCallback;
    IATextView *pWidget = nullptr;};



/**
 * Manage a floating point property that appears in a tree view as a text input
 * field with a pulldown menu of suggested values.
 */
class IAFloatChoiceController : public IATreeItemController
{
public:
    IAFloatChoiceController(const char *path, const char *label, IAFloatProperty &prop,
                            const char *unit, std::function<void()>&& cb,
                            Fl_Menu_Item *menu);
    virtual ~IAFloatChoiceController() override;
    virtual void build(Fl_Tree*, Type, int) override;
    virtual void propertyValueChanged(IAProperty*) override;

    static void wCallback(IAFloatChoiceView *w, IAFloatChoiceController *d);

    IAFloatProperty &pProperty;
    char *pUnit = nullptr;
    std::function<void()> pCallback;
    Fl_Menu_Item *pMenu = nullptr;
    IAFloatChoiceView *pWidget = nullptr;
};


/**
 * Manage an integer property that appears in a tree view as a pulldown menu.
 */
class IAChoiceController : public IATreeItemController
{
public:
    IAChoiceController(const char *path, const char *label, IAIntProperty &prop,
                       std::function<void()>&& cb, Fl_Menu_Item *menu);
    virtual ~IAChoiceController() override;
    virtual void build(Fl_Tree*, Type, int) override;
    virtual void propertyValueChanged(IAProperty*) override;

    static void wCallback(IAChoiceView *w, IAChoiceController *d);

    IAIntProperty &pProperty;
    Fl_Menu_Item *pMenu = nullptr;
    std::function<void()> pCallback;
    IAChoiceView *pWidget = nullptr;
};


/**
 * Manage a 3D vector property that appears in a tree view as up to three text input field.
 */
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
    virtual void propertyValueChanged(IAProperty*) override;

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


/**
 * Manage a text property that names a group preset.
 */
class IAPresetController : public IATreeItemController
{
    typedef IATreeItemController super;
public:
    IAPresetController(const char *path, const char *label,
                       IAPresetProperty &prop, std::function<void()>&& cb);
    virtual ~IAPresetController() override;
    virtual void build(Fl_Tree*, Type, int) override;
    virtual void propertyValueChanged(IAProperty*) override;

    static void wCallback(IAChoiceView *w, IAPresetController *d);
    void updateView(bool checkName);

    void buildMenu();
    IAPresetProperty &pProperty;
    Fl_Menu_Item *pMenu = nullptr;
    std::function<void()> pCallback;
    IAChoiceView *pWidget = nullptr;
    bool pPauseUpdates = false;
    bool pUnsaved = false;
};




typedef std::vector<IATreeItemController*> IAControllerList;


#endif /* IA_PRINTER_LIST_CONTROLLER_H */


