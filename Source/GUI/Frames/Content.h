#ifndef CONTENT_H
#define CONTENT_H

#include "wx/aui/aui.h"
#include <functional>
#include <map>
#include <vector>

class MainFrame;
class MouseHover;
class SourceEditor;

// Managing multiple editor windows with associated tabs.
class Content : public wxAuiNotebook
{
public:
	Content(MainFrame *parent);
	virtual ~Content();

	void SetWorkingDirectory(const wxString &dir)			{ workingDir = dir; }

	SourceEditor *GetSelectedSourceEditor()					{ return selectedEditor; }
	SourceEditor *GetSourceEditorWithDebugMarker()			{ return debugMarkedEditor; }
	SourceEditor *GetSourceEditor(const wxString &fileName);

	void UpdateSource(unsigned line, const wxString &fileName = wxEmptyString, bool moveDebugMarker = true);

	void DisableDebugMarker();
	void RemoveAllBreakpoints();
	void StopDebugging();

private:
	struct Mapping
	{
		wxString	original,
					mapped;
	};

	std::map<wxString, wxString>		sourceMapping;	// <original path, user supplied path>
	std::vector<Mapping>				inferMapping;	// The difference only.

	MainFrame							*host;
	wxString							workingDir;

	std::map<wxString, SourceEditor*>	sourceEditors;
	SourceEditor						*selectedEditor,
										*debugMarkedEditor;

	MouseHover							*mouseHover;

	SourceEditor *SelectSourceEditor(const wxString &fileName);
	SourceEditor *NewSourceEditor(const wxString &fileName, bool select);

	bool OpenFile(SourceEditor *editor, const wxString &fileName);

	void AddInferPath(const wxString &original, const wxString &mapped);
	bool InferPath(SourceEditor *editor, const wxString &fileName);

	// Iterate a function over all source editors. The function takes a SourceEditor reference as argument.
	void iterateSourceEditors(std::function<void (const SourceEditor&)> f) const;
	void iterateSourceEditors(std::function<void (SourceEditor&)> f);

	void RemoveSourceEditor(const wxString &fileName);

	void OnPageClose(wxAuiNotebookEvent &event);
	void OnPageChanged(wxAuiNotebookEvent &event);
	void OnMouseHovering(wxCommandEvent &event);

	wxDECLARE_EVENT_TABLE();
};

#endif // CONTENT_H
