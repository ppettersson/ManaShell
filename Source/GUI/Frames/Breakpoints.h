#ifndef BREAK_POINTS_H
#define BREAK_POINTS_H

#include "wx/wx.h"

class Breakpoints : public wxListBox
{
public:
	Breakpoints(wxWindow *parent);
	virtual ~Breakpoints();
};

#endif // BREAK_POINTS_H
