#ifndef DEBUGGER_DIALOG_H
#define DEBUGGER_DIALOG_H

#include "wx/wx.h"
#include <vector>

class Debugger;

class DebuggerDialog : public wxDialog
{
public:
	DebuggerDialog(wxWindow *parent, std::vector<Debugger *> &debuggers);
	virtual ~DebuggerDialog();

	Debugger *GetDebugger();

private:
	enum Constants
	{
		kDebuggerId		= 10000,
		kExecutableId,
		kExecutableBrowseId,
		kScriptId,
		kScriptBrowseId,
		kArgumentsId,
		kCustomId,
		kCommandId
	};

	wxChoice				*debuggerControl;
	wxTextCtrl				*executableControl;
	wxButton				*executableBrowse;
	wxTextCtrl				*scriptControl;
	wxButton				*scriptBrowse;
	wxTextCtrl				*argumentsControl;
	wxCheckBox				*customControl;
	wxTextCtrl				*commandControl;

	int						debugger;
	wxString				executable,
							script,
							arguments,
							command;
	bool					custom;

	std::vector<Debugger *>	debuggers;


	void SetDefaults();
	void UpdateCommand();

	void OnBrowse(wxCommandEvent &event);
	void OnCustomChanged(wxCommandEvent &event);
	void OnDebugger(wxCommandEvent &event);
	void OnLostFocus(wxFocusEvent &event);
	void OnUpdateUI(wxUpdateUIEvent &event);


	DECLARE_EVENT_TABLE()
	wxDECLARE_NO_COPY_CLASS(DebuggerDialog);
};

#endif // DEBUGGER_DIALOG_H
