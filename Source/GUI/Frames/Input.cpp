#include "Input.h"

Input::Input(wxWindow *parent)
	: wxTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(-1, 100), wxNO_BORDER)
{
}

Input::~Input()
{
}
