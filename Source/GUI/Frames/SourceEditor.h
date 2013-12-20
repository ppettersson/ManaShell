#ifndef SOURCE_EDITOR_H
#define SOURCE_EDITOR_H

#include "wx/wx.h"
#include "wx/stc/stc.h"
#include <map>

class MainFrame;

class SourceEditor : public wxStyledTextCtrl
{
public:
	SourceEditor(MainFrame *parent);
	virtual ~SourceEditor();

	void SetWorkingDirectory(const wxString &dir)		{ workingDir = dir; }

	bool Load(const wxString &fileName, unsigned line = 0, bool moveDebugMarker = true);

	void StopDebugging();
	void DisableDebugMarker();

	void AddBreakpoint(unsigned line);
	void RemoveBreakpoint(unsigned line);
	void RemoveAllBreakpoints();

	const wxString &GetCurrentFile() const				{ return currentFile; }


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

	enum Marker
	{
		kBreakpoint,
		kDebuggerNextLine
	};

	wxString						workingDir,
									currentFile;
	std::map<wxString, wxString>	sourceMapping;
	MainFrame						*host;

	void SetupMargins();
	void SetupHighlighting();

	void SetupCpp();
	void SetupPython();
	void SetupJava();

	bool OpenFile(const wxString &fileName);

	void OnMarginClick(wxStyledTextEvent &event);
};

#endif // SOURCE_EDITOR_H
