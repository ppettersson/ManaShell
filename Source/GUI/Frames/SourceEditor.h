#ifndef SOURCE_EDITOR_H
#define SOURCE_EDITOR_H

#include "wx/wx.h"
#include "wx/stc/stc.h"

class SourceEditor : public wxStyledTextCtrl
{
public:
	SourceEditor(wxWindow *parent);
	virtual ~SourceEditor();

	bool Load(const wxString &fileName, unsigned line = 0);


private:
	// Scintilla supports up to 5 margins.
	enum Margin
	{
		kMarker,		// Breakpoint icon, as well as the next line to be run
						// by the debugger when stepping.
		kLineNumber,	// Toggleable.
		kBlockFolding,	// Toggleable.
		kUnused0,
		kUnused1
	};

	wxString currentFile;

	void SetupMargins();
	void SetupHighlighting();
	void SetupCpp();

	void OnMarginClick(wxStyledTextEvent &event);
};

#endif // SOURCE_EDITOR_H
