#include "Breakpoints.h"

Breakpoints::Breakpoints(wxWindow *parent)
	: wxListBox(parent, wxID_ANY)
{
	// tmp
	Append("main.cpp, line 12");
	Append("fooBar.h, line 111");
}

Breakpoints::~Breakpoints()
{
}
