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


class IATreeItemView : public Fl_Group
{
public:
    IATreeItemView(IATreeItemController::Type t, int w, const char *label=nullptr);
protected:
    Fl_Box *pLabel;
};


class IALabelView : public IATreeItemView
{
public:
    IALabelView(IATreeItemController::Type t, int w, const char *label=nullptr, \
                const char *text=nullptr);
protected:
    Fl_Box *pText;
};


class IAFloatView : public IATreeItemView
{
public:
    IAFloatView(IATreeItemController::Type t, int w, const char *label=nullptr,
                const char *unit=nullptr);
    double value();
    void value(double v);
protected:
    static void choice_cb(Fl_Float_Input *w, void *u);
    Fl_Float_Input *pInput = nullptr;
};


class IATextView : public IATreeItemView
{
public:
    IATextView(IATreeItemController::Type t, int w, const char *label=nullptr,
               const char *text=nullptr);
    const char *value();
    void value(char const* v);
protected:
    static void choice_cb(Fl_Choice *w, void *u);
    Fl_Input *pInput = nullptr;
};


class IAFloatChoiceView : public IATreeItemView
{
public:
    IAFloatChoiceView(IATreeItemController::Type t, int w, const char *label,
                      Fl_Menu_Item *menu, const char *unit=nullptr);
    double value();
    void value(double v);
protected:
    static void choice_cb(Fl_Input_Choice *w, void *u);
    Fl_Input_Choice *pChoice = nullptr;
};


class IAChoiceView : public IATreeItemView
{
public:
    IAChoiceView(IATreeItemController::Type t, int w, const char *label,
                 Fl_Menu_Item *menu);
    int value();
    void value(int v);
protected:
    static void choice_cb(Fl_Choice *w, void *u);
    Fl_Choice *pChoice = nullptr;
};



#endif /* IA_TREE_VIEW_H */


