#include "../MainFrame.h"
#include "Input.h"

BEGIN_EVENT_TABLE(Input, wxTextCtrl)
	EVT_TEXT_ENTER(wxID_ANY, Input::OnEnter)
END_EVENT_TABLE()


Input::Input(MainFrame *parent)
	: wxTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition,
				 wxSize(-1, 100), wxNO_BORDER | wxTE_PROCESS_ENTER)
	, host(parent)
{
	Enable(false);
}

Input::~Input()
{
}

void Input::OnEnter(wxCommandEvent &event)
{
	wxString command = GetLineText(0);
	command += "\n";
	Clear();
	host->SendCommand(command);
}
