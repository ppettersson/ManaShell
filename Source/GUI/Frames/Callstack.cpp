#include "Callstack.h"

Callstack::Callstack(wxWindow *parent)
	: wxListBox(parent, wxID_ANY)
	, numFrames(0)
{
	// tmp.
	PushFrame("0x00000000");
	PushFrame("void main()");
	PushFrame("int calc(int, int)");
	PushFrame("bool test()");
	PopFrame();
	PushFrame("bool verify(const char *)");
}

Callstack::~Callstack()
{
}

void Callstack::PushFrame(const wxString &description)
{
	Insert(description, 0);
	SetSelection(0);
	++numFrames;
}

void Callstack::PopFrame()
{
	Delete(0);
	SetSelection(0);
	--numFrames;
}
