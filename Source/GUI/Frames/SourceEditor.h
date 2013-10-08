#ifndef SOURCE_EDITOR_H
#define SOURCE_EDITOR_H

#include "wx/wx.h"

class SourceEditor : public wxTextCtrl
{
public:
	SourceEditor(wxWindow *parent);
	virtual ~SourceEditor();
};

#endif // SOURCE_EDITOR_H
