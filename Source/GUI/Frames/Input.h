#ifndef INPUT_H
#define INPUT_H

#include "wx/wx.h"

class MainFrame;

class Input : public wxTextCtrl
{
public:
	Input(MainFrame *parent);
	virtual ~Input();


private:
	MainFrame	*host;


	void OnEnter(wxCommandEvent &event);

	wxDECLARE_EVENT_TABLE();
};

#endif // INPUT_H
