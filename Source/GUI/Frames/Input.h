#ifndef INPUT_H
#define INPUT_H

#include "wx/wx.h"

class Input : public wxTextCtrl
{
public:
	Input(wxWindow *parent);
	virtual ~Input();
};

#endif // INPUT_H
