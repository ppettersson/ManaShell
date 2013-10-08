#ifndef OUTPUT_H
#define OUTPUT_H

#include "wx/wx.h"

class Output : public wxTextCtrl
{
public:
	Output(wxWindow *parent);
	virtual ~Output();
};

#endif // OUTPUT_H
