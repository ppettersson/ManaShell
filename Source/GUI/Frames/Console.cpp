#include "../MainFrame.h"
#include "Console.h"

BEGIN_EVENT_TABLE(Console, wxPanel)
	EVT_TEXT_ENTER(wxID_ANY, Console::OnEnter)
END_EVENT_TABLE()


Console::Console(MainFrame *parent)
: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER)
, host(parent)
{
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	output = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTE_MULTILINE | wxTE_READONLY);
	sizer->Add(output, wxSizerFlags(1).Expand());

	input = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	sizer->Add(input, wxSizerFlags(0).Expand());

	SetSizer(sizer);
	sizer->SetSizeHints(this);
}

Console::~Console()
{
}

void Console::EnableInput(bool enable)
{
	input->Enable(enable);
}

void Console::EnableOuput(bool enable)
{
	output->Enable(enable);
}

void Console::SetFocusInput()
{
	input->SetFocus();
}

void Console::AppendText(const wxString &text)
{
	output->AppendText(text);
}

void Console::ClearOutput()
{
	output->Clear();
}

void Console::OnEnter(wxCommandEvent &event)
{
	wxString command = input->GetLineText(0);
	command += "\n";
	input->Clear();
	host->SendCommand(command, true);
}
