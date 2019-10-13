//
//  IAPrinter.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAController.h"

#include "view/IAGUIMain.h"
#include "view/IATreeItemView.h"
#include "property/IAProperty.h"

#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Preferences.H>
#include <FL/filename.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Tree_Item.H>
#include <FL/Fl_Float_Input.H>


/**
 * Create a controller.
 */
IAController::IAController()
{
}


/**
 * Destroy a controller.
 */
IAController::~IAController()
{
}


/**
 * Do nothing if a property changed.
 *
 * This function transfers infomation form the property to the view.
 */
void IAController::propertyValueChanged(IAProperty*)
{
}


/**
 * Set the Autodelete flag for a controller.
 *
 * If this flag is set, deleting a property will also delete this controller.
 * It's up to the controller to potetially delete the view as well.
 */
void IAController::autoDelete(bool v)
{
    pAutoDelete = v;
}


#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//


/**
 * Create a controller that triggers a callback, but is not associated to a view.
 *
 * \param[in] prop connect to an arbitrary property
 * \param[in] cb call this function when the property changes.
 */
IACallbackController::IACallbackController(IAProperty &prop, std::function<void()>&& cb)
:   pProperty( prop ),
    pCallback( cb )
{
    pProperty.attach(this);
}


/**
 * Destroy this controller.
 */
IACallbackController::~IACallbackController()
{
    pProperty.detach(this);
}


/**
 * This is called by the property whenever the property changes.
 */
void IACallbackController::propertyValueChanged(IAProperty*)
{
    if (pCallback)
        pCallback();
}


#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//


/**
 * Create a controller for an Fl_Tree.
 *
 * \param[in] path location of this tree item inside the tree hirarchy,
 *          may contain forward slashes.
 * \param[in] label a text that will appear to the left of the active widgets.
 */
IATreeItemController::IATreeItemController(const char *path, const char *label)
:   pPath(strdup(path)),
    pLabel(strdup(label))
{
}


/**
 * Destroy the instance.
 */
IATreeItemController::~IATreeItemController()
{
    if (pPath) ::free((void*)pPath);
    if (pLabel) ::free((void*)pLabel);
}


/**
 * Utility function to duplicate a simple Fl_Menu_Item list.
 *
 * \return Array of Fl_Menu_Item, must be free'd by the caller.
 */
Fl_Menu_Item *IATreeItemController::dup(Fl_Menu_Item const *src)
{
    Fl_Menu_Item const *s = src;
    int n = 1;
    while (s->label()!=nullptr) {
        s++; n++;
        // we assum that there are no submenus
    }
    Fl_Menu_Item *ret = (Fl_Menu_Item*)malloc(n*sizeof(Fl_Menu_Item));
    memmove(ret, src, n*sizeof(Fl_Menu_Item));
    return ret;
}


#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//


/**
 * Create a controller that manages a static text in an Fl_Tree.
 *
 * \param[in] path location of this tree item inside the tree hirarchy,
 *          may contain forward slashes.
 * \param[in] label a text that will appear to the left of the active widgets.
 * \param[in] text an additional text to the right of the label.
 */
IALabelController::IALabelController(const char *path, const char *label, const char *text)
:   IATreeItemController(path, label)
{
    if (text) pText = strdup(text);
}


/**
 * Destroy the instance.
 */
IALabelController::~IALabelController()
{
    if (pText) ::free((void*)pText);
}


/**
 * Build the view for this controller.
 *
 * \param[in] treeWidget create the view inside this Fl_Tree.
 * \param[in] t a preference style or toolbox style tree widget
 * \param[in] w width of the menu item in pixels
 */
void IALabelController::build(Fl_Tree *treeWidget, Type t, int w)
{
    if (!pWidget) {
        pWidget = new IALabelView(t, w, pLabel, pText);
        pWidget->tooltip(pTooltip);
    }
    pTreeItem = treeWidget->add(pPath);
    pTreeItem->close();
    pTreeItem->widget(pWidget);
}


#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//


/**
 * Create a controller that manages a floating point input field in an Fl_Tree.
 *
 * \param[in] path location of this tree item inside the tree hirarchy,
 *          may contain forward slashes.
 * \param[in] label a text that will appear to the left of the active widgets.
 * \param[in] prop control this property.
 * \param[in] unit a string showing the values units to the right of the input
 *          widget, can be nullptr.
 * \param[in] cb call this function when the property changes.
 */
IAFloatController::IAFloatController(const char *path, const char *label,
                                     IAFloatProperty &prop, const char *unit,
                                     std::function<void()>&& cb)
:   IATreeItemController(path, label),
pProperty(prop),
pUnit(strdup(unit?unit:"")),
pCallback(cb),
pWidget(nullptr)
{
    pProperty.attach(this);
}


/**
 * Destroy the instance.
 */
IAFloatController::~IAFloatController()
{
    pProperty.detach(this);
    if (pUnit) ::free((void*)pUnit);
    if (pWidget) delete pWidget;
}


/**
 * Manage callbacks from the view.
 */
void IAFloatController::wCallback(IAFloatView *w, IAFloatController *d)
{
    d->pProperty.set( w->value(), d );
    if (d->pCallback)
        d->pCallback();
}


/**
 * Build the view for this controller.
 *
 * \param[in] treeWidget create the view inside this Fl_Tree.
 * \param[in] t a preference style or toolbox style tree widget
 * \param[in] w width of the menu item in pixels
 */
void IAFloatController::build(Fl_Tree *treeWidget, Type t, int w)
{
    if (!pWidget) {
        pWidget = new IAFloatView(t, w, pLabel, pUnit);
        pWidget->value(pProperty());
        pWidget->callback((Fl_Callback*)wCallback, this);
        pWidget->tooltip(pTooltip);
    }
    pTreeItem = treeWidget->add(pPath);
    pTreeItem->close();
    pTreeItem->widget(pWidget);
}


/**
 * Called whenever the property changes, updates the associated widget.
 */
void IAFloatController::propertyValueChanged(IAProperty*)
{
    if (pWidget)
        pWidget->value(pProperty());
}


#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//


/**
 * Create a controller that manages a text input field in an Fl_Tree.
 *
 * \param[in] path location of this tree item inside the tree hirarchy,
 *          may contain forward slashes.
 * \param[in] label a text that will appear to the left of the active widgets.
 * \param[in] prop control this property.
 * \param[in] width of the input field, currently ignored
 * \param[in] unit a string showing the values units to the right of the input
 *          widget, can be nullptr.
 * \param[in] cb call this function when the property changes.
 */
IATextController::IATextController(const char *path, const char *label, IATextProperty &prop,
                                   int wdt, const char *unit,
                                   std::function<void()>&& cb)
:   IATreeItemController(path, label),
pProperty(prop),
pWdt(wdt),
pUnit(strdup(unit?unit:"")),
pCallback(cb),
pWidget(nullptr)
{
    pProperty.attach(this);
}


/**
 * Destroy the instance.
 */
IATextController::~IATextController()
{
    pProperty.detach(this);
    if (pUnit) ::free((void*)pUnit);
    if (pWidget) delete pWidget;
}


/**
 * Manage callbacks from the view.
 */
void IATextController::wCallback(IATextView *w, IATextController *d)
{
    d->pProperty.set( w->value(), d );
    if (d->pCallback)
        d->pCallback();
}


/**
 * Build the view for this controller.
 *
 * \param[in] treeWidget create the view inside this Fl_Tree.
 * \param[in] t a preference style or toolbox style tree widget
 * \param[in] w width of the menu item in pixels
 */
void IATextController::build(Fl_Tree *treeWidget, Type t, int w)
{
    if (!pWidget) {
        pWidget = new IATextView(t, w, pLabel, pUnit);
        pWidget->value(pProperty());
        pWidget->callback((Fl_Callback*)wCallback, this);
        pWidget->tooltip(pTooltip);
    }
    pTreeItem = treeWidget->add(pPath);
    pTreeItem->close();
    pTreeItem->widget(pWidget);
}


/**
 * Called whenever the property changes, updates the associated widget.
 */
void IATextController::propertyValueChanged(IAProperty*)
{
    if (pWidget)
        pWidget->value(pProperty());
}


#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//


/**
 * Create a controller that manages a floating point input field and
 * pulldown menu in an Fl_Tree.
 *
 * \param[in] path location of this tree item inside the tree hirarchy,
 *          may contain forward slashes.
 * \param[in] label a text that will appear to the left of the active widgets.
 * \param[in] prop control this property.
 * \param[in] unit a string showing the values units to the right of the input
 *          widget, can be nullptr.
 * \param[in] cb call this function when the property changes.
 * \param[in] menu list of menu items with presets for the text field, menu
 *          will be duplicated and managed by the class
 */
IAFloatChoiceController::IAFloatChoiceController(const char *path, const char *label,
                                                 IAFloatProperty &prop, const char *unit,
                                                 std::function<void()>&& cb, Fl_Menu_Item *menu)
:   IATreeItemController(path, label),
pProperty(prop),
pUnit(strdup(unit?unit:"")),
pCallback(cb),
pMenu(dup(menu)),
pWidget(nullptr)
{
    pProperty.attach(this);
}


/**
 * Destroy the instance.
 */
IAFloatChoiceController::~IAFloatChoiceController()
{
    pProperty.detach(this);
    if (pUnit) ::free((void*)pUnit);
    if (pMenu) ::free((void*)pMenu);
    if (pWidget) delete pWidget;
}


/**
 * Manage callbacks from the view.
 */
void IAFloatChoiceController::wCallback(IAFloatChoiceView *w, IAFloatChoiceController *d)
{
    d->pProperty.set( w->value(), d );
    if (d->pCallback)
        d->pCallback();
}


/**
 * Build the view for this controller.
 *
 * \param[in] treeWidget create the view inside this Fl_Tree.
 * \param[in] t a preference style or toolbox style tree widget
 * \param[in] w width of the menu item in pixels
 */
void IAFloatChoiceController::build(Fl_Tree *treeWidget, Type t, int w)
{
    if (!pWidget) {
        pWidget = new IAFloatChoiceView(t, w, pLabel, pMenu, pUnit);
        pWidget->value( pProperty() );
        pWidget->callback((Fl_Callback*)wCallback, this);
        pWidget->tooltip(pTooltip);
    }
    pTreeItem = treeWidget->add(pPath);
    pTreeItem->close();
    pTreeItem->widget(pWidget);
}


/**
 * Called whenever the property changes, updates the associated widget.
 */
void IAFloatChoiceController::propertyValueChanged(IAProperty*)
{
    if (pWidget)
        pWidget->value(pProperty());
}


#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//


/**
 * Create a controller that manages a pulldown menu in an Fl_Tree.
 *
 * \param[in] path location of this tree item inside the tree hirarchy,
 *          may contain forward slashes.
 * \param[in] label a text that will appear to the left of the active widgets.
 * \param[in] prop control this property.
 * \param[in] cb call this function when the property changes.
 * \param[in] menu list of menu items with presets for the text field, menu
 *          will be duplicated and managed by the class
 */
IAChoiceController::IAChoiceController(const char *path, const char *label, IAIntProperty &prop,
                                       std::function<void()>&& cb, Fl_Menu_Item *menu)
:   IATreeItemController(path, label),
    pProperty(prop),
    pMenu(dup(menu)),
    pCallback(cb),
    pWidget(nullptr)
{
    pProperty.attach(this);
}


/**
 * Destroy the instance.
 */
IAChoiceController::~IAChoiceController()
{
    pProperty.detach(this);
    if (pMenu) ::free((void*)pMenu);
    if (pWidget) delete pWidget;
}


/**
 * Manage callbacks from the view.
 */
void IAChoiceController::wCallback(IAChoiceView *w, IAChoiceController *d)
{
    d->pProperty.set( w->value(), d );
    if (d->pCallback)
        d->pCallback();
}


/**
 * Build the view for this controller.
 *
 * \param[in] treeWidget create the view inside this Fl_Tree.
 * \param[in] t a preference style or toolbox style tree widget
 * \param[in] w width of the menu item in pixels
 */
void IAChoiceController::build(Fl_Tree *treeWidget, Type t, int w)
{
    if (!pWidget) {
        pWidget = new IAChoiceView(t, w, pLabel, pMenu);
        /** \bug compare to user_data() in the menu list to find the right entry */
        pWidget->value(pProperty());
        pWidget->callback((Fl_Callback*)wCallback, this);
        pWidget->tooltip(pTooltip);
    }
    pTreeItem = treeWidget->add(pPath);
    pTreeItem->close();
    pTreeItem->widget(pWidget);
}


/**
 * Called whenever the property changes, updates the associated widget.
 */
void IAChoiceController::propertyValueChanged(IAProperty*)
{
    if (pWidget)
        pWidget->value(pProperty());
}


#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//


/**
 * Create a controller that manages a floating point input field and
 * pulldown menu in an Fl_Tree.
 *
 * \param[in] path location of this tree item inside the tree hirarchy,
 *          may contain forward slashes.
 * \param[in] label a text that will appear to the left of the active widgets.
 * \param[in] text an additional text to the right of the label.
 * \param[in] prop control this property.
 * \param[in] xLabel a text that will appear to the left of the x input.
 * \param[in] xUnits a text that will appear to the right of the x input, can be nullptr.
 * \param[in] yLabel a text that will appear to the left of the x input, can be nullptr to show only the x value.
 * \param[in] yUnits a text that will appear to the right of the x input, can be nullptr.
 * \param[in] zLabel a text that will appear to the left of the x input, can be nullptr to show only the x and y value.
 * \param[in] zUnits a text that will appear to the right of the x input, can be nullptr.
 * \param[in] cb call this function when the property changes.
 */
IAVectorController::IAVectorController(const char *path, const char *label, const char *text,
                                       IAVectorProperty &prop,
                                       char const* xLabel, char const* xUnits,
                                       char const* yLabel, char const* yUnits,
                                       char const* zLabel, char const* zUnits,
                                       std::function<void()>&& cb)
:   IATreeItemController(path, label),
    pProperty(prop),
    pCallback(cb),
    pXLabel(xLabel), pXUnits(xUnits),
    pYLabel(yLabel), pYUnits(yUnits),
    pZLabel(zLabel), pZUnits(zUnits)
{
    if (text) pText = strdup(text);
    pProperty.attach(this);
}


/**
 * Destroy the instance.
 */
IAVectorController::~IAVectorController()
{
    pProperty.detach(this);
    if (pLabelWidget) delete pLabelWidget;
    if (pXWidget) delete pXWidget;
    if (pYWidget) delete pYWidget;
    if (pZWidget) delete pZWidget;
    if (pText) ::free((void*)pText);
    if (pXPath) ::free((void*)pXPath);
    if (pYPath) ::free((void*)pYPath);
    if (pZPath) ::free((void*)pZPath);
}


/**
 * Build the view for this controller.
 *
 * \param[in] treeWidget create the view inside this Fl_Tree.
 * \param[in] t a preference style or toolbox style tree widget
 * \param[in] w width of the menu item in pixels
 */
void IAVectorController::build(Fl_Tree *treeWidget, Type t, int w)
{
    if (!pLabelWidget) {
        pLabelWidget = new IALabelView(t, w, pLabel, pText);
        pLabelWidget->tooltip(pTooltip);
    }
    pTreeItem = treeWidget->add(pPath);
    pTreeItem->close();
    pTreeItem->widget(pLabelWidget);

    if (pXLabel && !pXWidget) {
        pXWidget = new IAFloatView(t, w, pXLabel, pXUnits);
        pXWidget->value(pProperty().x());
        pXWidget->callback((Fl_Callback*)wCallback, this);
    }
    if (pXWidget) {
        if (!pXPath) pXPath = strdup(Fl_Preferences::Name("%s/x", pPath));
        pTreeItem = treeWidget->add(pXPath);
        pTreeItem->widget(pXWidget);
    }

    if (pYLabel && !pYWidget) {
        pYWidget = new IAFloatView(t, w, pYLabel, pYUnits);
        pYWidget->value(pProperty().y());
        pYWidget->callback((Fl_Callback*)wCallback, this);
    }
    if (pYWidget) {
        if (!pYPath) pYPath = strdup(Fl_Preferences::Name("%s/y", pPath));
        pTreeItem = treeWidget->add(pYPath);
        pTreeItem->widget(pYWidget);
    }

    if (pZLabel && !pZWidget) {
        pZWidget = new IAFloatView(t, w, pZLabel, pZUnits);
        pZWidget->value(pProperty().z());
        pZWidget->callback((Fl_Callback*)wCallback, this);
    }
    if (pZWidget) {
        if (!pZPath) pZPath = strdup(Fl_Preferences::Name("%s/z", pPath));
        pTreeItem = treeWidget->add(pZPath);
        pTreeItem->widget(pZWidget);
    }
}


/**
 * Called whenever the property changes, updates the associated widget.
 */
void IAVectorController::propertyValueChanged(IAProperty*)
{
    if (pXWidget) pXWidget->value(pProperty().x());
    if (pYWidget) pYWidget->value(pProperty().y());
    if (pZWidget) pZWidget->value(pProperty().z());
}


/**
 * Manage callbacks from the view.
 */
void IAVectorController::wCallback(IAFloatView *w, IAVectorController *d)
{
    IAVector3d v;
    if (d->pXWidget) v.x( d->pXWidget->value() );
    if (d->pYWidget) v.y( d->pYWidget->value() );
    if (d->pZWidget) v.z( d->pZWidget->value() );
    d->pProperty.set( v, d );
    if (d->pCallback)
        d->pCallback();
}


#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//


/**
 * Create a controller that manages presets for a group of Properties.
 *
 * \param[in] path location of this tree item inside the tree hirarchy,
 *          may contain forward slashes.
 * \param[in] label a text that will appear to the left of the active widgets.
 * \param[in] prop control this property.
 * \param[in] cb call this function when the property changes.
 */
IAPresetController::IAPresetController(const char *path, const char *label,
                                       IAPresetProperty &prop, std::function<void()>&& cb)
:   super(path, label),
    pProperty( prop ),
    pCallback( cb )
{
    pProperty.attach(this);
    // super:: already attaches to the property event list
    /// \todo how will we make all the other properties in the group signal this if they changed?
    // add a function in the property to attach all other property controllers? But what if they were not created yet?
}


/**
 * Destroy the instance.
 */
IAPresetController::~IAPresetController()
{
    pProperty.detach(this);
}


/**
 * Build the view for this controller.
 *
 * \param[in] treeWidget create the view inside this Fl_Tree.
 * \param[in] t a preference style or toolbox style tree widget
 * \param[in] w width of the menu item in pixels
 */
void IAPresetController::build(Fl_Tree *treeWidget, Type t, int w)
{
    buildMenu();
    pProperty.attachClients(this);
    if (!pWidget) {
        pWidget = new IAChoiceView(t, w, pLabel, pMenu);
        updateView(true);
        pWidget->callback((Fl_Callback*)wCallback, this);
        pWidget->tooltip(pTooltip);
    }
    pTreeItem = treeWidget->add(pPath);
    pTreeItem->close();
    pTreeItem->widget(pWidget);
}


/**
 * Set the view to the value in pProperty.
 */
void IAPresetController::updateView(bool checkName)
{
    bool found;
    if (checkName)
        found = pWidget->value(pProperty());
    else
        found = false;

    if (found) {
        pUnsaved = false;
        pMenu[0].hide();
        pMenu[1].hide();
        pMenu[2].hide();
        pMenu[3].show();
    } else {
        pUnsaved = false;
        char buf[FL_PATH_MAX];
        ::free((void*)pMenu[0].label());
        snprintf(buf, sizeof(buf), "* %s", pProperty());
        pMenu[0].label(strdup(buf));
        pMenu[0].show();
        pMenu[1].show();
        pMenu[2].show();
        pMenu[3].hide();
        pWidget->value(0);
        pWidget->redraw();
    }
}


/**
 * Called whenever the property changes, updates the associated widget.
 */
void IAPresetController::propertyValueChanged(IAProperty *p)
{
    if (p==&pProperty) {
        /// update the view (clients must be updated by property.set()).
        // a different preset was selected by some source
        updateView(true);
        // pProperty.load();
    } else if (p==(IAProperty*)1) {
        // disable updates for now
        pPauseUpdates = true;
    } else if (p==(IAProperty*)2) {
        // disable updates for now
        pPauseUpdates = false;
    } else {
        // update the menu: one of the values inside the preset group has changed
        if (pPauseUpdates) {
            ;
        } else {
            updateView(false);
        }
    }
}


/**
 * Manage callbacks from the view.
 */
void IAPresetController::wCallback(IAChoiceView *w, IAPresetController *d)
{
    int i = w->value();
    if (i==0) { // <not saved>
        /// \todo implement what happens if we choose this item (does anything happen?)
    } else if (i==1) { // overwrite preset...
        // ask user, if overwriting is ok
        if (fl_choice("Please confirm that your want to overwrite this preset.", "Overwrite", "Cancel", nullptr))
            return;
        // save the preset under the new name
        d->pProperty.save();
        d->buildMenu();
        d->pWidget->menu(d->pMenu);
        d->updateView(true);
    } else if (i==2) { // save as preset...
        /// \todo implement a way to name and save this new preset
        // ask user for a new name
        const char *newLabel = fl_input("Please enter a label for this new preset");
        if (!newLabel) {
            d->updateView(false);
            return;
        }
        /// \todo check if the label name conforms to FL_Prefrences tags, or escape it!
        // what do we do if the name overrides a builtin preset?
        // check if that name exists
        bool found = d->pWidget->value(newLabel);
        d->pWidget->value(0);
        // ask user, if overwriting is ok
        if (found)
            if (fl_choice("A preset by that name already exists.", "Overwrite", "Cancel", nullptr))
                return;
        // save the preset under the new name
        d->pProperty.save(newLabel);
        // rebuild the menu
        d->buildMenu();
        d->pWidget->menu(d->pMenu);
        d->updateView(true);
        // set the value to the new name menu item
    } else if (i==3) { // remove this preset...
        // check if this is a bultin preset and cancel if it is
        if (d->pMenu[5].label()==nullptr) {
            fl_message("The last remaining preset can not be deleted.");
            return;
        }
        if (fl_choice("Please confirm that you want to permanently remove\n"
                      "the preset \"%s\".", "Remove", "Cancel", nullptr, d->pProperty()))
            return;
        // delete preset from file
        d->pProperty.erase();
        // rebuild menu
        d->buildMenu();
        d->pWidget->menu(d->pMenu);
        d->pProperty.set(d->pMenu[4].label());
        // set next best value
    } else {
        d->pProperty.set( d->pMenu[i].label(), d );
        // d->pProperty.load(); // property.set() will load the settings for us
        if (d->pCallback)
            d->pCallback();
        d->updateView(true);
    }
}


/**
 * Build a menu from the list of availabel presets.
 */
void IAPresetController::buildMenu()
{
    if (pMenu) ::free((void*)pMenu); /** \bug and free the labels! */
    std::vector< std::string > presetList;
    pProperty.listPresets(presetList);
    int n = (int)presetList.size();
    pMenu = (Fl_Menu_Item*)calloc(n+5, sizeof(Fl_Menu_Item));
    pMenu[0] = { strdup("<not saved>"), 0, nullptr, (void*)0, FL_MENU_INACTIVE, 0, 0, 11 };
    pMenu[1] = { strdup("overwrite preset"), 0, nullptr, (void*)1, 0, 0, 0, 11 };
    pMenu[2] = { strdup("save preset as..."), 0, nullptr, (void*)2, FL_MENU_DIVIDER, 0, 0, 11 };
    pMenu[3] = { strdup("remove this preset..."), 0, nullptr, (void*)3, FL_MENU_DIVIDER|FL_MENU_INVISIBLE, 0, 0, 11 };
    for (int i=0; i<n; i++) {
        pMenu[i+4] = { strdup(presetList[i].c_str()), 0, nullptr, (void*)(fl_intptr_t)(i+4), 0, 0, 0, 11 };
    }
}


