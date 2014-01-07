#ifndef WATCH_H
#define WATCH_H

#include "wx/wx.h"
#include "wx/grid.h"
#include <vector>

class MainFrame;

class Watch : public wxGrid
{
public:
	Watch(MainFrame *parent);
	virtual ~Watch();

	bool GetNext(unsigned index, wxString &result);
	void Update(unsigned index, const wxString &value, const wxString &type);

	void ClearAll();

private:
	struct Variable	// ...or Expression.
	{
		wxString			name,
							value,
							type;

		Variable()								{ }
		Variable(const wxString &n) : name(n)	{ }
	};

	MainFrame				*host;
	unsigned				numVariables;
	std::vector<Variable>	variables;


	void AddVariable(const wxString &name = "", const wxString &value = "", const wxString &type = "");

	void AutoSizeLastCol(int winWidth);

	void OnChanged(wxGridEvent &event);
	void OnSize(wxSizeEvent &event);
	void OnColSize(wxGridSizeEvent &event);

	wxDECLARE_EVENT_TABLE();
};

#endif // WATCH_H
