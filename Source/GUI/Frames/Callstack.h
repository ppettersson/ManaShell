#ifndef CALL_STACK_H
#define CALL_STACK_H

#include "wx/wx.h"

class Callstack : public wxListBox
{
public:
	Callstack(wxWindow *parent);
	virtual ~Callstack();

	void PushFrame(const wxString &description);
	void PopFrame();

private:
	unsigned	numFrames;
};

#endif // CALL_STACK_H
