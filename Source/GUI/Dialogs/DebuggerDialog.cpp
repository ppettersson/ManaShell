#include "DebuggerDialog.h"
#include "wx/valgen.h"
#include "wx/valnum.h"
#include "wx/valtext.h"

IMPLEMENT_CLASS(DebuggerDialog, wxDialog)

BEGIN_EVENT_TABLE(DebuggerDialog, wxDialog)
	EVT_UPDATE_UI(wxID_OK, DebuggerDialog::OnUpdateUI)
END_EVENT_TABLE()


DebuggerDialog::DebuggerDialog(wxWindow *parent)
	: wxDialog(parent, wxID_ANY, "Select Debugger Plugin", wxDefaultPosition, wxDefaultSize,
				wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxTAB_TRAVERSAL)
{
	SetDefaults();

	wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(topSizer);

	topSizer->Add(CreateTab(this), 1, wxGROW | wxALL, 5);
	topSizer->SetSizeHints(this);
}

wxPanel *DebuggerDialog::CreateTab(wxWindow *parent)
{
	wxPanel *panel = new wxPanel(this);

	wxBoxSizer *verticalSizer = new wxBoxSizer(wxVERTICAL);
	panel->SetSizer(verticalSizer);
	{
		int spacing = wxLEFT | wxRIGHT | wxUP;

		wxChoice *debuggerPlugin = new wxChoice(panel, kDebuggerId);
		debuggerPlugin->Append("Python");
		debuggerPlugin->Append("Custom");
		debuggerPlugin->Select(0);
		debuggerPlugin->SetValidator(wxGenericValidator(&debugger));
		debuggerPlugin->Bind(wxEVT_COMMAND_CHOICE_SELECTED, &DebuggerDialog::OnDebugger, this);
		verticalSizer->Add(debuggerPlugin, 1, wxGROW | spacing, 5);

		wxBoxSizer *executableSizer = new wxBoxSizer(wxHORIZONTAL);
		verticalSizer->Add(executableSizer, 1, wxGROW | spacing, 5);
		{
			wxStaticText *description = new wxStaticText(panel, wxID_STATIC, "Executable:");
			executableSizer->Add(description, 0, wxALIGN_CENTER_VERTICAL | spacing, 5);

			executableControl = new wxTextCtrl(panel, kExecutableId);
			executableControl->SetValidator(wxGenericValidator(&executable));
			executableControl->SetValue(executable);
			executableControl->Bind(wxEVT_KILL_FOCUS, &DebuggerDialog::OnLostFocus, this);
			executableSizer->Add(executableControl, 1, wxGROW | spacing, 5);

			executableBrowse = new wxButton(panel, kExecutableBrowseId, "...");
			executableBrowse->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DebuggerDialog::OnBrowse, this);
			executableSizer->Add(executableBrowse, 0, wxALIGN_CENTER_VERTICAL | spacing, 5);
		}

		wxBoxSizer *scriptSizer = new wxBoxSizer(wxHORIZONTAL);
		verticalSizer->Add(scriptSizer, 1, wxGROW | spacing, 5);
		{
			wxStaticText *description = new wxStaticText(panel, wxID_STATIC, "Script to debug:");
			scriptSizer->Add(description, 0, wxALIGN_CENTER_VERTICAL | spacing, 5);

			scriptControl = new wxTextCtrl(panel, kScriptId);
			scriptControl->SetValidator(wxGenericValidator(&script));
			scriptControl->SetValue(script);
			scriptControl->Bind(wxEVT_KILL_FOCUS, &DebuggerDialog::OnLostFocus, this);
			scriptSizer->Add(scriptControl, 1, wxGROW | spacing, 5);

			scriptBrowse = new wxButton(panel, kScriptBrowseId, "...");
			scriptBrowse->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DebuggerDialog::OnBrowse, this);
			scriptSizer->Add(scriptBrowse, 0, wxALIGN_CENTER_VERTICAL | spacing, 5);
		}

		wxBoxSizer *argumentsSizer = new wxBoxSizer(wxHORIZONTAL);
		verticalSizer->Add(argumentsSizer, 1, wxGROW | spacing, 5);
		{
			wxStaticText *description = new wxStaticText(panel, wxID_STATIC, "Arguments to script:");
			argumentsSizer->Add(description, 0, wxALIGN_CENTER_VERTICAL | spacing, 5);

			argumentsControl = new wxTextCtrl(panel, kArgumentsId);
			argumentsControl->SetValidator(wxGenericValidator(&arguments));
			argumentsControl->SetValue(arguments);
			argumentsControl->Bind(wxEVT_KILL_FOCUS, &DebuggerDialog::OnLostFocus, this);
			argumentsSizer->Add(argumentsControl, 1, wxGROW | spacing, 5);
		}

		wxBoxSizer *commandSizer = new wxBoxSizer(wxHORIZONTAL);
		verticalSizer->Add(commandSizer, 1, wxGROW | spacing, 5);
		{
			wxCheckBox *check = new wxCheckBox(panel, kCustomId, "Custom");
			check->SetValidator(wxGenericValidator(&custom));
			check->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &DebuggerDialog::OnCustomChanged, this);
			commandSizer->Add(check, 0, wxALIGN_CENTER_VERTICAL | spacing, 5);

			commandControl = new wxTextCtrl(panel, kCommandId);
			commandControl->SetValidator(wxGenericValidator(&command));
			commandControl->Enable(false);
			commandControl->SetValue(command);
			commandControl->Bind(wxEVT_KILL_FOCUS, &DebuggerDialog::OnLostFocus, this);
			commandSizer->Add(commandControl, 1, wxGROW | spacing, 5);
		}

		verticalSizer->AddSpacer(15);

		wxButton *run = new wxButton(panel, wxID_OK, "Run");
		verticalSizer->Add(run, 0, wxALIGN_RIGHT | wxALL, 5);
	}

	return panel;
}

void DebuggerDialog::SetDefaults()
{
	debugger	= kPython;
	executable	= wxEmptyString;
	script		= wxEmptyString;
	arguments	= wxEmptyString;
	custom		= false;

	switch (debugger)
	{
	case kPython:
		executable	= "python";
		script		= "script.py";
		break;
	}

	UpdateCommand();
}

void DebuggerDialog::UpdateCommand()
{
	switch (debugger)
	{
	case kPython:
		command = executable;
		command += " -i -m pdb ";
		command += script;
		break;
	};
}

void DebuggerDialog::OnBrowse(wxCommandEvent &event)
{
	// Browse for the file.
	wxString result = wxFileSelector("Choose a file", wxEmptyString, wxEmptyString, wxEmptyString,
		"All files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	// If the dialog was cancelled then this is empty.
	if (!result.IsEmpty())
		switch (event.GetId())
		{
		case kExecutableBrowseId:	executableControl->SetValue(result);	break;
		case kScriptBrowseId:		scriptControl->SetValue(result);		break;
		}
}

void DebuggerDialog::OnCustomChanged(wxCommandEvent &event)
{
	bool custom = event.IsChecked();

	executableControl->Enable(!custom);
	executableBrowse->Enable(!custom);
	scriptControl->Enable(!custom);
	scriptBrowse->Enable(!custom);
	argumentsControl->Enable(!custom);

	commandControl->Enable(custom);

	if (!custom)
	{
		UpdateCommand();
		commandControl->SetValue(command);
	}
}

void DebuggerDialog::OnDebugger(wxCommandEvent &event)
{
}

void DebuggerDialog::OnLostFocus(wxFocusEvent &event)
{
	switch (event.GetId())
	{
	case kExecutableId:
	case kScriptId:
	case kArgumentsId:
		UpdateCommand();
		commandControl->SetValue(command);
		break;
	}
}

void DebuggerDialog::OnUpdateUI(wxUpdateUIEvent &event)
{
	if (event.GetId() == wxID_OK)
	{
		bool enable = false;
		switch (debugger)
		{
		case kPython:
			if (custom)
				enable = !commandControl->GetValue().IsEmpty();
			else
				enable = !executableControl->GetValue().IsEmpty() &&
						 !scriptControl->GetValue().IsEmpty();
			break;
		}
		event.Enable(enable);
	}
}
