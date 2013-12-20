#ifndef CONSOLE_H
#define CONSOLE_H

#include "wx/wx.h"

class MainFrame;

// Panel contains a text control to show the debugger output and a text control
// for debugger command input.
class Console : public wxPanel
{
public:
	Console(MainFrame *parent);
	virtual ~Console();

	void EnableInput(bool enable);
	void EnableOuput(bool enable);

	void SetFocusInput();

	void AppendText(const wxString &text);

	void ClearOutput();

private:
	MainFrame	*host;
	wxTextCtrl	*output;
	wxTextCtrl	*input;

	void OnEnter(wxCommandEvent &event);

	wxDECLARE_EVENT_TABLE();
};

#endif // CONSOLE_H
