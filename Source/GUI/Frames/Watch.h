#ifndef WATCH_H
#define WATCH_H

#include "wx/wx.h"
#include "wx/grid.h"

class Watch : public wxGrid
{
public:
	Watch(wxWindow *parent);
	virtual ~Watch();

	void AddVariable(const wxString &name, const wxString &value, const wxString &type);


private:
	unsigned	numVariables;
};

#endif // WATCH_H
