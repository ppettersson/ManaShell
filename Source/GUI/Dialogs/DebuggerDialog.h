#ifndef DEBUGGER_DIALOG_H
#define DEBUGGER_DIALOG_H

#include "wx/wx.h"
#include <vector>

class Debugger;

// Set up how the debugger should be called when starting a new session.
class DebuggerDialog : public wxDialog
{
public:
	DebuggerDialog(wxWindow *parent, std::vector<Debugger *> &debuggers);
	virtual ~DebuggerDialog();

	Debugger *GetDebugger();


private:
	// Controller Id's used when creating event handlers.
	enum Constants
	{
		kDebuggerId		= 10000,
		kExecutableId,
		kExecutableBrowseId,
		kScriptId,
		kScriptBrowseId,
		kArgumentsId,
		kCustomId,
		kCommandId,
		kWorkingDirId,
		kWorkingDirBrowseId
	};

	// Access to the controls to make it easier to enable or disable them
	// directly.
	wxChoice				*debuggerControl;		// Which plugin to use.
	wxTextCtrl				*executableControl;		// Where to find the debugger program.
	wxButton				*executableBrowse;		// Use a file browser to find the debugger program.
	wxTextCtrl				*scriptControl;			// Where to find the script/program to debug.
	wxButton				*scriptBrowse;			// Use a file browser to find the script/program to debug.
	wxTextCtrl				*argumentsControl;		// What arguments to pass to the script/program.
	wxCheckBox				*customControl;			// If the command should be overriden.
	wxTextCtrl				*commandControl;		// Overridden or generated command line.
	wxTextCtrl				*workingDirControl;		// Working dir when launching the debugger.
	wxButton				*workingDirBrowse;		// Use a directory browser to find the working directory.

	// Current "committed" state.
	Debugger				*debugger;
	wxString				executable,
							script,
							arguments,
							command,
							workingDir;
	bool					custom;

	// All the different plugins.
	std::vector<Debugger *>	debuggers;


	void SetDefaults();
	void UpdateCommand();
	void TransferDataToDebugger();
	void TransferDataFromDebugger();

	// Event handlers.
	void OnBrowse(wxCommandEvent &event);
	void OnBrowseDir(wxCommandEvent &event);
	void OnCustomChanged(wxCommandEvent &event);
	void OnDebugger(wxCommandEvent &event);
	void OnLostFocus(wxFocusEvent &event);
	void OnUpdateUI(wxUpdateUIEvent &event);
	void OnOK(wxCommandEvent &event);


	DECLARE_EVENT_TABLE()
	wxDECLARE_NO_COPY_CLASS(DebuggerDialog);
};

#endif // DEBUGGER_DIALOG_H
