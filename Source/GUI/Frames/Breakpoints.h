#ifndef BREAK_POINTS_H
#define BREAK_POINTS_H

#include "wx/wx.h"
#include <vector>

class MainFrame;

class Breakpoints : public wxListBox
{
public:
	enum ToggleResult
	{
		kAdded,
		kRemoved
	};

	Breakpoints(MainFrame *parent);
	virtual ~Breakpoints();

	ToggleResult ToggleBreak(const wxString &fileName, unsigned line);
	void ClearAllBreakpoints();


private:
	enum Constants
	{
		kAddBreakpoint	= 10001
	};

	struct Break
	{
		wxString		fileName;
		unsigned		line;

		Break()
			: line(0)
		{
		}

		Break(const wxString &f, unsigned l)
			: fileName(f)
			, line(l)
		{
		}
	};

	MainFrame			*host;
	std::vector<Break>	breaks;


	void UpdateUI();
	void OnDClick(wxCommandEvent &event);
	void OnContextMenu(wxContextMenuEvent &event);
	void OnAddBreakpoint(wxCommandEvent &event);


	wxDECLARE_EVENT_TABLE();
};

#endif // BREAK_POINTS_H
