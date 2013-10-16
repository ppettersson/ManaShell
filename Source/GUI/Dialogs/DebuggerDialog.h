#ifndef DEBUGGER_DIALOG_H
#define DEBUGGER_DIALOG_H

#include "wx/wx.h"

class DebuggerDialog : public wxDialog
{
public:
	enum Debugger
	{
		kPython,
		kCustom
	};


	DebuggerDialog(wxWindow *parent);


	Debugger GetDebugger() const			{ return (Debugger)debugger; }
	const wxString &GetCommand() const		{ return command; }


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


	int			debugger;
	wxString	executable,
				script,
				arguments,
				command;
	bool		custom;

	wxTextCtrl	*executableControl,
				*scriptControl,
				*argumentsControl,
				*commandControl;
	wxButton	*executableBrowse,
				*scriptBrowse;


	wxPanel *CreateTab(wxWindow *parent);
	void SetDefaults();
	void UpdateCommand();

	void OnBrowse(wxCommandEvent &event);
	void OnCustomChanged(wxCommandEvent &event);
	void OnDebugger(wxCommandEvent &event);
	void OnLostFocus(wxFocusEvent &event);
	void OnUpdateUI(wxUpdateUIEvent &event);


	DECLARE_CLASS(DebuggerDialog)
	DECLARE_EVENT_TABLE()
};

#endif // DEBUGGER_DIALOG_H
