//
//  IAPrinter.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "view/IATreeItemView.h"

#include <FL/Fl_Box.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Float_Input.H>


#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//


IATreeItemView::IATreeItemView(IATreeItemController::Type t, int w, const char *label)
:   Fl_Group(0, 0, w, t==IATreeItemController::kSetting?13:15)
{
    box(FL_FLAT_BOX);
    begin();
    pLabel = new Fl_Box(0, 0, w/2-4, h());
    pLabel->labelsize(h()-2);
    pLabel->label(label);
    pLabel->align(FL_ALIGN_INSIDE|FL_ALIGN_LEFT);
    end();
}


#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//


IALabelView::IALabelView(IATreeItemController::Type t, int w, const char *label, const char *text)
:   IATreeItemView(t, w, label)
{
    begin();
    pText = new Fl_Box(w/2, 0, w/2-4, h(), text);
    pText->labelsize(h()-2);
    pText->align(FL_ALIGN_INSIDE|FL_ALIGN_LEFT);
    end();
}


#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//


IAFloatView::IAFloatView(IATreeItemController::Type t, int w, const char *label, const char *unit)
:   IATreeItemView(t, w, label)
{
    begin();
    pInput = new Fl_Float_Input(w/2, 0, 65, h(), unit);
    pInput->textsize(h()-2);
    pInput->labelsize(h()-2);
    pInput->align(FL_ALIGN_RIGHT);
    pInput->callback((Fl_Callback*)choice_cb);
    end();
}


double IAFloatView::value()
{
    return atof(pInput->value());
}


void IAFloatView::value(double v)
{
    char buf[80];
    snprintf(buf, 80, "%.2f", v);
    pInput->value(buf);
}


void IAFloatView::choice_cb(Fl_Float_Input *w, void *u)
{
    w->parent()->do_callback();
}


#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//


IATextView::IATextView(IATreeItemController::Type t, int w, const char *label, const char *unit)
:   IATreeItemView(t, w, label)
{
    begin();
    pInput = new Fl_Input(w/2, 0, h()*12, h(), unit);
    pInput->textsize(h()-2);
    pInput->labelsize(h()-2);
    pInput->align(FL_ALIGN_RIGHT);
    pInput->callback((Fl_Callback*)choice_cb);
    end();
}


const char *IATextView::value()
{
    return pInput->value();
}


void IATextView::value(char const* v)
{
    pInput->value(v);
}


void IATextView::choice_cb(Fl_Choice *w, void *u)
{
    w->parent()->do_callback();
}


#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//


IAFloatChoiceView::IAFloatChoiceView(IATreeItemController::Type t, int w,
                                     const char *label, Fl_Menu_Item *menu,
                                     const char *unit)
:   IATreeItemView(t, w, label)
{
    begin();
    pChoice = new Fl_Input_Choice(w/2, 0, 65, h(), unit);
    pChoice->textsize(h()-2);
    pChoice->labelsize(h()-2);
    pChoice->align(FL_ALIGN_RIGHT);
    pChoice->menu(menu);
    pChoice->callback((Fl_Callback*)choice_cb);
    end();
}


double IAFloatChoiceView::value()
{
    return atof(pChoice->value());
}


void IAFloatChoiceView::value(double v)
{
    char buf[80];
    snprintf(buf, 80, "%.2f", v);
    pChoice->value(buf);
}


void IAFloatChoiceView::choice_cb(Fl_Input_Choice *w, void *u)
{
    IAFloatChoiceView *c = (IAFloatChoiceView*)w->parent();
    //c->value( c->value() ); // reinterprete the string that was set by the pulldown
    c->do_callback();
}


#ifdef __APPLE__
#pragma mark -
#endif
//============================================================================//


IAChoiceView::IAChoiceView(IATreeItemController::Type t, int w, const char *label,
                           Fl_Menu_Item *menu)
:   IATreeItemView(t, w, label)
{
    begin();
    pChoice = new Fl_Choice(w/2, 0, h()*7, h());
    // FIXME: calculate sizes using menu entries and unit string length
    pChoice->textsize(h()-2);
    pChoice->labelsize(h()-2);
    pChoice->align(FL_ALIGN_RIGHT);
    pChoice->menu(menu);
    pChoice->callback((Fl_Callback*)choice_cb);
    end();
}


int IAChoiceView::value()
{
    return (int)(fl_intptr_t)(pChoice->mvalue()->user_data());
}


void IAChoiceView::value(int v)
{
    const Fl_Menu_Item *m = pChoice->menu();
    void *p = (void*)(fl_intptr_t)v;
    for (int i=0; ; i++) {
        if (m[i].label()==nullptr) break;
        if (m[i].user_data()==p) {
            pChoice->value(m+i);
            break;
        }
    }
}


void IAChoiceView::choice_cb(Fl_Choice *w, void *u)
{
    w->parent()->do_callback();
}


