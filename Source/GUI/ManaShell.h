#ifndef MANA_SHELL_H
#define MANA_SHELL_H

#include "wx/wx.h"

class Debugger;
class MainFrame;

// The main application object.
class ManaShellApp : public wxApp
{
public:
	ManaShellApp();
	virtual ~ManaShellApp();


private:
	Debugger	*debugger;
	MainFrame	*mainFrame;

	virtual bool OnInit();
	virtual void OnInitCmdLine(wxCmdLineParser &parser);
	virtual bool OnCmdLineParsed(wxCmdLineParser &parser);
};

wxDECLARE_APP(ManaShellApp);

#endif // MANA_SHELL_H
