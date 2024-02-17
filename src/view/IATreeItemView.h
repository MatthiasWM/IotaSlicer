//
//  IATreeView.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_TREE_VIEW_H
#define IA_TREE_VIEW_H


#include "Iota.h"

#include "controller/IAController.h"

#include <FL/Fl_Group.H>


class Fl_Input_Choice;
class Fl_Choice;
class Fl_Box;
class Fl_Float_Input;
class Fl_Input;


/**
 * The base class for all views.
 *
 * A view is a UI representation of a property. Views are connected to
 * properties through controllers. Multiple views can each connect through their
 * controller to a single property. When a property changes, all controllers
 * are signaled, and all views are updated.
 *
 * \see IAController, IAProperty
 */
class IAView : public Fl_Group
{
    typedef Fl_Group super;
public:
    IAView(int x, int y, int w, int h, const char *label=nullptr)
    : super(x, y, w, h, label) { }
};


/**
 * This is the base class for various editable views in a tree widget.
 */
class IATreeItemView : public IAView
{
    typedef IAView super;
public:
    IATreeItemView(IATreeItemController::Type t, int w, const char *label=nullptr);
protected:
    Fl_Box *pLabel;
};


/**
 * This view creates a mathing label and secondary text for tree views.
 */
class IALabelView : public IATreeItemView
{
    typedef IATreeItemView super;
public:
    IALabelView(IATreeItemController::Type t, int w, const char *label=nullptr, \
                const char *text=nullptr);
protected:
    Fl_Box *pText;
};


/**
 * This view provides a numeric editor through a text input field for tree views.
 */
class IAFloatView : public IATreeItemView
{
    typedef IATreeItemView super;
public:
    IAFloatView(IATreeItemController::Type t, int w, const char *label=nullptr,
                const char *unit=nullptr);
    double value();
    void value(double v);
protected:
    static void choice_cb(Fl_Float_Input *w, void *u);
    Fl_Float_Input *pInput = nullptr;
};


/**
 * This view provides a text editor for tree views.
 */
class IATextView : public IATreeItemView
{
    typedef IATreeItemView super;
public:
    IATextView(IATreeItemController::Type t, int w, const char *label=nullptr,
               const char *text=nullptr);
    const char *value();
    void value(char const* v);
protected:
    static void choice_cb(Fl_Choice *w, void *u);
    Fl_Input *pInput = nullptr;
};


/**
 * This view edits floating point values via a text field and a pulldown menu.
 *
 * The pulldown menu is great for providing useful presets. The text field
 * will interprete all consecutive numeric characters from the start of the
 * menu label.
 */
class IAFloatChoiceView : public IATreeItemView
{
    typedef IATreeItemView super;
public:
    IAFloatChoiceView(IATreeItemController::Type t, int w, const char *label,
                      Fl_Menu_Item *menu, const char *unit=nullptr);
    double value();
    void value(double v);
protected:
    static void choice_cb(Fl_Input_Choice *w, void *u);
    Fl_Input_Choice *pChoice = nullptr;
};


/**
 * This view provides a pulldown menu with integer output for tree views.
 */
class IAChoiceView : public IATreeItemView
{
    typedef IATreeItemView super;
public:
    IAChoiceView(IATreeItemController::Type t, int w, const char *label,
                 Fl_Menu_Item *menu);
    int value();
    void value(int v);
    bool value(char const* v);
    void menu(Fl_Menu_Item*);
protected:
    static void choice_cb(Fl_Choice *w, void *u);
    Fl_Choice *pChoice = nullptr;
};



#endif /* IA_TREE_VIEW_H */


