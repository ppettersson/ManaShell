#ifndef WATCH_H
#define WATCH_H

#include "wx/wx.h"

class wxGrid;

class Watch : public wxPanel
{
public:
	Watch(wxWindow *parent);
	virtual ~Watch();


private:
	wxGrid	*grid;
};

#endif // WATCH_H
