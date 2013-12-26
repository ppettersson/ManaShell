#include "../../Plugins/Debugger.h"
#include "DebuggerDialog.h"
#include "wx/valgen.h"
#include "wx/valnum.h"
#include "wx/valtext.h"
#include "wx/statline.h"

BEGIN_EVENT_TABLE(DebuggerDialog, wxDialog)
	EVT_UPDATE_UI(wxID_OK, DebuggerDialog::OnUpdateUI)
	EVT_BUTTON(wxID_OK, DebuggerDialog::OnOK)
END_EVENT_TABLE()


DebuggerDialog::DebuggerDialog(wxWindow *parent, std::vector<Debugger *> &d)
	: wxDialog(parent, wxID_ANY, "Select Debugger Plugin", wxDefaultPosition,
			   wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
	, debuggers(d)
{
	wxASSERT(debuggers.size());

	// Allow validation to break through static box sizers.
	SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);

	// Flexible layout with sizers.
	wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
	{
		// Drop down for all the plugins.
		wxArrayString debuggerNames;
		for (std::vector<Debugger *>::iterator i = debuggers.begin(); i != debuggers.end(); ++i)
			debuggerNames.Add((*i)->GetName());
		debuggerControl = new wxChoice(this, kDebuggerId, wxDefaultPosition, wxDefaultSize, debuggerNames);
		debuggerControl->Bind(wxEVT_CHOICE, &DebuggerDialog::OnDebugger, this);
		topSizer->Add(debuggerControl, 0, wxGROW | wxALL, 5);

		// Where to find the debugger program and what arguments to pass.
		wxStaticBoxSizer *parametersSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, "Parameters:"), wxVERTICAL);
		topSizer->Add(parametersSizer, 0, wxGROW | wxALL, 5);
		{
			wxBoxSizer *executableSizer = new wxBoxSizer(wxHORIZONTAL);
			parametersSizer->Add(executableSizer, 0, wxGROW | wxALL, 5);
			{
				wxStaticText *staticText = new wxStaticText(parametersSizer->GetStaticBox(), wxID_STATIC, "Executable:");
				executableSizer->Add(staticText, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

				executableControl = new wxTextCtrl(parametersSizer->GetStaticBox(), kExecutableId);
				executableControl->SetValidator(wxTextValidator(wxFILTER_NONE, &executable));
				executableControl->Bind(wxEVT_KILL_FOCUS, &DebuggerDialog::OnLostFocus, this);
				executableSizer->Add(executableControl, 1, wxGROW | wxALL, 5);

				executableBrowse = new wxButton(parametersSizer->GetStaticBox(), kExecutableBrowseId, "...");
				executableBrowse->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DebuggerDialog::OnBrowse, this);
				executableSizer->Add(executableBrowse, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
			}

			wxBoxSizer *scriptSizer = new wxBoxSizer(wxHORIZONTAL);
			parametersSizer->Add(scriptSizer, 0, wxGROW | wxALL, 5);
			{
				wxStaticText *staticText = new wxStaticText(parametersSizer->GetStaticBox(), wxID_STATIC, "Script:");
				scriptSizer->Add(staticText, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

				scriptControl = new wxTextCtrl(parametersSizer->GetStaticBox(), kScriptId);
				scriptControl->SetValidator(wxTextValidator(wxFILTER_NONE, &script));
				scriptControl->Bind(wxEVT_KILL_FOCUS, &DebuggerDialog::OnLostFocus, this);
				scriptSizer->Add(scriptControl, 1, wxGROW | wxALL, 5);

				scriptBrowse = new wxButton(parametersSizer->GetStaticBox(), kScriptBrowseId, "...");
				scriptBrowse->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DebuggerDialog::OnBrowse, this);
				scriptSizer->Add(scriptBrowse, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
			}

			wxBoxSizer *argumentsSizer = new wxBoxSizer(wxHORIZONTAL);
			parametersSizer->Add(argumentsSizer, 0, wxGROW | wxALL, 5);
			{
				wxStaticText *description = new wxStaticText(parametersSizer->GetStaticBox(), wxID_STATIC, "Arguments:");
				argumentsSizer->Add(description, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

				argumentsControl = new wxTextCtrl(parametersSizer->GetStaticBox(), kArgumentsId);
				argumentsControl->SetValidator(wxTextValidator(wxFILTER_NONE, &arguments));
				argumentsControl->Bind(wxEVT_KILL_FOCUS, &DebuggerDialog::OnLostFocus, this);
				argumentsSizer->Add(argumentsControl, 1, wxGROW | wxALL, 5);
			}
		}

		// The resulting command line and an option to override it.
		wxStaticBoxSizer *commandSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, "Command:"), wxVERTICAL);
		topSizer->Add(commandSizer, 0, wxGROW | wxALL, 5);
		{
			wxBoxSizer *customSizer = new wxBoxSizer(wxHORIZONTAL);
			commandSizer->Add(customSizer, 0, wxGROW | wxALL, 5);
			{
				customControl = new wxCheckBox(commandSizer->GetStaticBox(), kCustomId, "Custom");
				customControl->SetValidator(wxGenericValidator(&custom));
				customControl->Bind(wxEVT_CHECKBOX, &DebuggerDialog::OnCustomChanged, this);
				customSizer->Add(customControl, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

				commandControl = new wxTextCtrl(commandSizer->GetStaticBox(), kCommandId);
				commandControl->Enable(false);
				commandControl->SetValidator(wxTextValidator(wxFILTER_NONE, &command));
				commandControl->Bind(wxEVT_KILL_FOCUS, &DebuggerDialog::OnLostFocus, this);
				customSizer->Add(commandControl, 1, wxGROW | wxALL, 5);
			}

			wxBoxSizer *workingDirSizer = new wxBoxSizer(wxHORIZONTAL);
			commandSizer->Add(workingDirSizer, 0, wxGROW | wxALL, 5);
			{
				wxStaticText *description = new wxStaticText(commandSizer->GetStaticBox(), wxID_STATIC, "Working Dir:");
				workingDirSizer->Add(description, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

				workingDirControl = new wxTextCtrl(commandSizer->GetStaticBox(), kWorkingDirId);
				workingDirControl->SetValidator(wxTextValidator(wxFILTER_NONE, &workingDir));
				workingDirControl->Bind(wxEVT_KILL_FOCUS, &DebuggerDialog::OnLostFocus, this);
				workingDirSizer->Add(workingDirControl, 1, wxGROW | wxALL, 5);

				workingDirBrowse = new wxButton(commandSizer->GetStaticBox(), kWorkingDirBrowseId, "...");
				workingDirBrowse->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DebuggerDialog::OnBrowseDir, this);
				workingDirSizer->Add(workingDirBrowse, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
			}
		}

		// Spacer.
		topSizer->Add(5, 5, 1, wxGROW | wxALL, 5);

		// Regular dialog dismissal.
		wxStdDialogButtonSizer *dialogButtons = new wxStdDialogButtonSizer();
		dialogButtons->AddButton(new wxButton(this, wxID_OK, "OK"));
		dialogButtons->AddButton(new wxButton(this, wxID_CANCEL, "Cancel"));
		dialogButtons->Realize();
		topSizer->Add(dialogButtons, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
	}

	SetDefaults();
	
	topSizer->SetMinSize(wxSize(600, -1));
	SetSizerAndFit(topSizer);
	Center();
}

DebuggerDialog::~DebuggerDialog()
{
}

Debugger *DebuggerDialog::GetDebugger()
{
	// Delete the ones that aren't going to be used.
	for (std::vector<Debugger *>::iterator i = debuggers.begin(); i != debuggers.end(); ++i)
		if (*i != debugger)
			delete *i;
	debuggers.clear();

	// Return the one that was selected.
	return debugger;
}

void DebuggerDialog::SetDefaults()
{
	debugger	= debuggers[0];
	custom		= false;

	TransferDataFromDebugger();

	command = debugger->GetCommand();
}

void DebuggerDialog::UpdateCommand()
{
	TransferDataToDebugger();

	command = debugger->GetCommand();
}

void DebuggerDialog::TransferDataToDebugger()
{
	debugger->SetExecutable(executable);
	debugger->SetScript(script);
	debugger->SetArguments(arguments);
	debugger->SetUseCustomCommand(custom);
	if (custom)
		debugger->SetCustomCommand(command);
	debugger->SetWorkingDir(workingDir);
}

void DebuggerDialog::TransferDataFromDebugger()
{
	executable	= debugger->GetExecutable();
	script		= debugger->GetScript();
	arguments	= debugger->GetArguments();
	workingDir	= debugger->GetWorkingDir();
}

void DebuggerDialog::OnBrowse(wxCommandEvent &event)
{
	// Browse for the file.
	wxString result = wxFileSelector("Choose a file", wxEmptyString, wxEmptyString, wxEmptyString,
		"All files (*.*)|*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	// If the dialog was cancelled then this is empty.
	if (!result.IsEmpty())
	{
		bool update = false;

		switch (event.GetId())
		{
		case kExecutableBrowseId:
			executable = result;
			update = true;
			break;

		case kScriptBrowseId:
			script = result;
			update = true;
			break;
		}

		if (update)
		{
			UpdateCommand();
			TransferDataToWindow();
		}
	}
}

void DebuggerDialog::OnBrowseDir(wxCommandEvent &event)
{
	// Browse for the directory.
	wxString result = wxDirSelector("Choose a directory");

	// If the dialog was cancelled then this is empty.
	if (!result.IsEmpty())
	{
		workingDir = result;
		TransferDataToWindow();
	}
}

void DebuggerDialog::OnCustomChanged(wxCommandEvent &event)
{
	custom = event.IsChecked();

	debugger->SetUseCustomCommand(custom);

	// Custom is mutually exclusive with setting parameters.
	executableControl->Enable(!custom);
	executableBrowse->Enable(!custom);
	scriptControl->Enable(!custom);
	scriptBrowse->Enable(!custom);
	argumentsControl->Enable(!custom);

	commandControl->Enable(custom);

	if (!custom)
	{
		UpdateCommand();
		TransferDataToWindow();
	}
}

void DebuggerDialog::OnDebugger(wxCommandEvent &event)
{
	if (debuggers[event.GetInt()] != debugger)
	{
		// Save the current select in case the old debugger is selected again.
		TransferDataToDebugger();

		// Retrieve new values from the new debugger.
		debugger = debuggers[event.GetInt()];
		TransferDataFromDebugger();

		// Update UI.
		wxCommandEvent dummy;
		dummy.SetInt(0);
		OnCustomChanged(dummy);

		TransferDataToWindow();
	}
}

void DebuggerDialog::OnLostFocus(wxFocusEvent &event)
{
	bool update = false;

	switch (event.GetId())
	{
	case kExecutableId:
		executable = executableControl->GetValue();
		update = true;
		break;

	case kScriptId:
		script = scriptControl->GetValue();
		update = true;
		break;

	case kArgumentsId:
		arguments = argumentsControl->GetValue();
		update = true;
		break;

	case kCommandId:
		command = commandControl->GetValue();
		update = true;
		break;

	case kWorkingDirId:
		workingDir = workingDirControl->GetValue();
		break;
	}

	if (update)
	{
		UpdateCommand();
		TransferDataToWindow();
	}

	event.Skip();
}

void DebuggerDialog::OnUpdateUI(wxUpdateUIEvent &event)
{
	switch (event.GetId())
	{
	case wxID_OK:
		if (custom)
			event.Enable(!commandControl->GetValue().IsEmpty());
		else
			event.Enable(!executableControl->GetValue().IsEmpty() && !scriptControl->GetValue().IsEmpty());
		break;
	}
}

void DebuggerDialog::OnOK(wxCommandEvent &event)
{
	TransferDataToDebugger();

	event.Skip();
}
