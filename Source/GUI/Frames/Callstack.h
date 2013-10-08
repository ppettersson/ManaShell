#ifndef CALL_STACK_H
#define CALL_STACK_H

#include "wx/wx.h"

class Callstack : public wxListBox
{
public:
	Callstack(wxWindow *parent);
	virtual ~Callstack();
};

#endif // CALL_STACK_H
