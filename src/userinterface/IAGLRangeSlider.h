//
// IAGLRangeSlider.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_GL_RANGE_SLIDER_H
#define IA_GL_RANGE_SLIDER_H

#include <FL/Fl_Slider.H>

/**
 * A slider that can be used to set a range of units within a bigger
 * container.
 *
 * In Ioat, we use this to display a number of printing preview layers within
 * the entire build volume height or within the model height.
 */
class IAGLRangeSlider : public Fl_Slider {

protected:

    // these allow subclasses to put the slider in a smaller area:
    int handle(int, int, int, int, int);
    void draw();

public:

    double lowValue() { return maximum()-pHighValue; }
    double highValue() { return maximum()-pLowValue; }
    void lowValue(double v) { pHighValue = maximum()-v; redraw(); }
    void highValue(double v) { pLowValue = maximum()-v; redraw(); }

    double pLowValue = 0.0;
    double pHighValue = 0.0;

    int baseHeight = 13;
    int btn = 0;

    int handle(int);
    IAGLRangeSlider(int X,int Y,int W,int H, const char *L = 0);
    //  Fl_Slider(uchar t,int X,int Y,int W,int H, const char *L);
    //
    //  int scrollvalue(int pos,int size,int first,int total);
    //  void bounds(double a, double b);
    //
    //  /**
    //    Get the dimensions of the moving piece of slider.
    //  */
    //  float slider_size() const {return slider_size_;}
    //
    //  /**
    //    Set the dimensions of the moving piece of slider. This is
    //    the fraction of the size of the entire widget. If you set this
    //    to 1 then the slider cannot move.  The default value is .08.
    //
    //    For the "fill" sliders this is the size of the area around the
    //    end that causes a drag effect rather than causing the slider to
    //    jump to the mouse.
    //  */
    //  void slider_size(double v);
    //
    //  /** Gets the slider box type. */
    //  Fl_Boxtype slider() const {return (Fl_Boxtype)slider_;}
    //
    //  /** Sets the slider box type. */
    //  void slider(Fl_Boxtype c) {slider_ = c;}
};

#endif /* IA_GL_RANGE_SLIDER_H */
