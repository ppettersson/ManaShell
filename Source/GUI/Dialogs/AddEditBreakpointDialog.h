#ifndef ADD_EDIT_BREAKPOINT_DIALOG_H
#define ADD_EDIT_BREAKPOINT_DIALOG_H

#include "wx/wx.h"
#include "wx/spinctrl.h"

class AddEditBreakpointDialog : public wxDialog
{
public:
	AddEditBreakpointDialog(wxWindow *parent, const wxString &fileName = wxEmptyString, int lineNumber = 0);
	virtual ~AddEditBreakpointDialog();

	const wxString &GetFileName() const 	{ return fileName; }
	int GetLineNumber() const 				{ return lineNumber; }


private:
	// Controller Id's used when creating event handlers.
	enum Constants
	{
		kFileNameId 		= 10000,
		kFileNameBrowseId,
		kLineNumberId
	};

	// Direct access to the controls.
	wxTextCtrl	*fileNameControl;

	// Current "committed" state.
	wxString	fileName;
	int			lineNumber;


	// Event handlers.
	void OnBrowse(wxCommandEvent &event);
	void OnUpdateUI(wxUpdateUIEvent &event);


	DECLARE_EVENT_TABLE()
	wxDECLARE_NO_COPY_CLASS(AddEditBreakpointDialog);
};

#endif // ADD_EDIT_BREAKPOINT_DIALOG_H
