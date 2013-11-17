#ifndef MANA_SHELL_H
#define MANA_SHELL_H

#include "wx/wx.h"

class MainFrame;

// The main application object.
class ManaShellApp : public wxApp
{
public:
	ManaShellApp();
	virtual ~ManaShellApp();


private:
	MainFrame	*mainFrame;

	virtual bool OnInit();
};

wxDECLARE_APP(ManaShellApp);

#endif // MANA_SHELL_H
