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



//class IACtrlTreeItemFloat;
//class IALabelView;
//class IAFloatView;
//class IATextView;
//class IAFloatProperty;
//class IAFloatChoiceView;
//class IAIntProperty;
//class IAChoiceView;
//
//struct Fl_Menu_Item;
class Fl_Input_Choice;
class Fl_Choice;
class Fl_Box;
class Fl_Float_Input;
class Fl_Input;
//class Fl_Tree;
//class Fl_Tree_Item;
//class Fl_Preferences;
//struct Fl_Menu_Item;
//
//
//
//
//
//#include "IAController.h"
//
//#include "view/IAGUIMain.h"
//#include "property/IAProperty.h"
//
//#include <FL/Fl_Menu_Item.H>
//#include <FL/Fl_Preferences.H>
//#include <FL/filename.H>
//#include <FL/Fl_Tree.H>
//#include <FL/Fl_Tree_Item.H>
//#include <FL/Fl_Float_Input.H>


class IALabelView : public Fl_Group
{
public:
    IALabelView(IATreeViewController::Type t, int w, const char *label=nullptr);
    Fl_Box *pLabel;
    Fl_Box *pText;
};


class IAFloatView : public Fl_Group
{
public:
    IAFloatView(IATreeViewController::Type t, int w, const char *label=nullptr);
    double value();
    void value(double v);
    static void choice_cb(Fl_Float_Input *w, void *u);
    Fl_Box *pLabel = nullptr;
    Fl_Float_Input *pInput = nullptr;
};


class IATextView : public Fl_Group
{
public:
    IATextView(IATreeViewController::Type t, int w, const char *label=nullptr);
    const char *value();
    void value(char const* v);
    static void choice_cb(Fl_Choice *w, void *u);
    Fl_Box *pLabel = nullptr;
    Fl_Input *pInput = nullptr;
};


class IAFloatChoiceView : public Fl_Group
{
public:
    IAFloatChoiceView(IATreeViewController::Type t, int w, const char *label=nullptr);
    double value();
    void value(double v);
    static void choice_cb(Fl_Input_Choice *w, void *u);
    Fl_Box *pLabel = nullptr;
    Fl_Input_Choice *pChoice = nullptr;
};


class IAChoiceView : public Fl_Group
{
public:
    IAChoiceView(int x, int y, int w, int h, const char *label=nullptr);
    int value();
    void value(int v);
    static void choice_cb(Fl_Choice *w, void *u);
    Fl_Choice *pChoice = nullptr;
};



#endif /* IA_TREE_VIEW_H */


