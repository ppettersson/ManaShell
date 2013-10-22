#include "Output.h"

Output::Output(wxWindow *parent)
	: wxTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition,
				 wxSize(450, -1), wxNO_BORDER | wxTE_MULTILINE |
				 wxTE_READONLY)
{
	SetFont(wxFont(8, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
}

Output::~Output()
{
}
