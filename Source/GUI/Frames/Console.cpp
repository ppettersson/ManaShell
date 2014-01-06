#include "../MainFrame.h"
#include "Console.h"

Console::Console(MainFrame *parent)
	: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER)
	, host(parent)
	, historyPos(0)
{
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

	output = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
							wxDefaultSize, wxNO_BORDER | wxTE_MULTILINE | wxTE_READONLY);
	sizer->Add(output, wxSizerFlags(1).Expand());

	input = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
							wxDefaultSize, wxTE_PROCESS_ENTER);
	input->Connect(wxEVT_TEXT_ENTER, wxTextEventHandler(Console::OnEnter), NULL, this);
	input->Connect(wxEVT_KEY_DOWN, wxKeyEventHandler(Console::OnKeyDown), NULL, this);
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
	if (!command.IsEmpty())
	{
		history.push_back(command);
		historyPos = history.size();
		input->Clear();
		command += "\n";
		host->SendCommand(command, true);
	}
	else if (historyPos < history.size())
	{
		history.erase(history.begin() + historyPos);
	}

	historyPos = history.size();
}

void Console::OnKeyDown(wxKeyEvent &event)
{
	size_t oldPos = historyPos;

	switch (event.GetKeyCode())
	{
	case WXK_UP:
		historyPos -= (historyPos > 0 ? 1 : 0);
		break;
	case WXK_DOWN:
		historyPos += (historyPos < history.size() ? 1 : 0);
		break;
	default:
		event.Skip();
		return;
	}

	if (historyPos != oldPos)
	{
		if (oldPos < history.size())
		{
			// Update history.
			wxString command = input->GetLineText(0);
			if (command.IsEmpty())
			{
				history.erase(history.begin() + oldPos);
				if (historyPos > oldPos)
					historyPos = oldPos;
			}
			else
			{
				history[oldPos] = input->GetLineText(0);
			}
		}

		input->SetValue(historyPos < history.size() ? history[historyPos] : "");
	}
}
