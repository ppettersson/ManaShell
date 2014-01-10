#include "../MainFrame.h"
#include "../MouseHover.h"
#include "Breakpoints.h"
#include "Content.h"
#include "SourceEditor.h"
#include "wx/artprov.h"
#include "wx/filename.h"

BEGIN_EVENT_TABLE(Content, wxAuiNotebook)
	EVT_AUINOTEBOOK_PAGE_CHANGED(wxID_ANY, Content::OnPageChanged)
	EVT_AUINOTEBOOK_PAGE_CLOSE(wxID_ANY, Content::OnPageClose)
	EVT_COMMAND(wxID_ANY, MOUSE_HOVERING, Content::OnMouseHovering)
END_EVENT_TABLE()

Content::Content(MainFrame *parent)
	: wxAuiNotebook(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER |
		wxAUI_NB_CLOSE_ON_ALL_TABS | wxAUI_NB_WINDOWLIST_BUTTON |
		wxAUI_NB_SCROLL_BUTTONS)
	, host(parent)
	, selectedEditor(NULL)
	, debugMarkedEditor(NULL)
{
	SetArtProvider(new wxAuiSimpleTabArt);

	mouseHover = new MouseHover(this, 1000);
}

Content::~Content()
{
	delete mouseHover;
}

SourceEditor *Content::GetSourceEditor(const wxString &fileName)
{
	auto it = sourceEditors.find(fileName);
	return (it != sourceEditors.end() ? it->second : NULL);
}

void Content::UpdateSource(unsigned line, const wxString &fileName, bool moveDebugMarker)
{
	SourceEditor *sourceEditor = (wxEmptyString == fileName) ? debugMarkedEditor : SelectSourceEditor(fileName);

	if (!sourceEditor)
	{
		sourceEditor = NewSourceEditor(fileName, true);
		if (!OpenFile(sourceEditor, fileName))
		{
			// Failed to open file.
			RemoveSourceEditor(fileName);
			DeletePage(GetPageIndex(sourceEditor));
			sourceEditor = NULL;
		}
		else
		{
			// Add existing breakpoints.
			std::vector<int> lines;
			host->GetBreakpoints()->GetLines(fileName, lines);
			for (std::vector<int>::const_iterator it = lines.begin(), endIt = lines.end(); it != endIt; ++it)
				sourceEditor->AddBreakpoint(*it);
		}
	}

	if (moveDebugMarker && sourceEditor != debugMarkedEditor)
	{
		if (debugMarkedEditor)
			debugMarkedEditor->DisableDebugMarker();
		debugMarkedEditor = sourceEditor;
	}

	if (sourceEditor)
		sourceEditor->GotoLine(line, moveDebugMarker);
}

void Content::DisableDebugMarker()
{
	if (debugMarkedEditor)
	{
		debugMarkedEditor->DisableDebugMarker();
		debugMarkedEditor = NULL;
	}
}

void Content::RemoveAllBreakpoints()
{
	auto f = [](SourceEditor &se) { se.RemoveAllBreakpoints(); };
	iterateSourceEditors(f);
}

void Content::StopDebugging()
{
	DeleteAllPages();
	sourceEditors.clear();
	workingDir			= "";
	debugMarkedEditor 	= NULL;
	selectedEditor 		= NULL;
}

SourceEditor *Content::SelectSourceEditor(const wxString &fileName)
{
	SourceEditor *editor = GetSourceEditor(fileName);
	if (selectedEditor != editor)
	{
		selectedEditor = editor;
		SetSelection(GetPageIndex(editor));
	}

	return editor;
}

SourceEditor *Content::NewSourceEditor(const wxString &fileName, bool select)
{
	wxASSERT(sourceEditors.find(fileName) == sourceEditors.end());
	sourceEditors[fileName] = selectedEditor = new SourceEditor(host);

	Freeze();
	wxBitmap pageBmp = wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER, wxSize(16, 16));
	AddPage(selectedEditor, wxFileNameFromPath(fileName), select, pageBmp);
	SetPageToolTip(GetPageIndex(selectedEditor), fileName);
	Thaw();

	return selectedEditor;
}

bool Content::OpenFile(SourceEditor *editor, const wxString &fileName)
{
	// First try to open it as is.
	if (wxFileExists(fileName) && editor->Load(fileName))
		return true;

	// If it was a relative path, then try to figure out the absolute
	// path from the working directory.
	wxFileName fn;
	fn.Assign(fileName);
	if (fn.IsRelative())
	{
		if (fn.MakeAbsolute(workingDir))
		{
			wxString fullPath = fn.GetFullPath();
			if (wxFileExists(fullPath) && editor->Load(fullPath))
				return true;
		}
	}

	// Check if we already have this file mapped.
	std::map<wxString, wxString>::iterator i = sourceMapping.find(fileName);
	if (i != sourceMapping.end())
	{
		const wxString &fullPath = i->second;
		if (wxFileExists(fullPath) && editor->Load(fullPath))
			return true;
	}

	// Check if we can infer the path from previous source mappings.
	if (InferPath(editor, fileName))
		return true;

	// If that fails as well, then ask the user to locate the file.
	wxString result = wxFileSelector(
		wxString::Format("Please locate this file: %s", fileName),
		fn.GetPath(), fn.GetName(), fn.GetExt(),
		"All files (*.*)|*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	// If the dialog was cancelled then this is empty.
	if (!result.IsEmpty())
	{
		if (wxFileExists(result) && editor->Load(result))
		{
			// Store this mapping in case it's requested again.
			sourceMapping[fileName] = result;

			// Store the relative change to help with infering other files.
			AddInferPath(fileName, result);
			return true;
		}
	}

	wxMessageBox(wxString::Format("Failed to open file: %s", fileName), "Error",
				 wxOK | wxCENTRE | wxICON_ERROR, GetParent());
	return false;
}

void Content::AddInferPath(const wxString &original, const wxString &mapped)
{
	// Try to match up as much as possible of the path.
	int originalEnd = original.length();
	int mappedEnd = mapped.length();

	while (originalEnd > 0 && mappedEnd > 0 &&
			(original[originalEnd] == mapped[mappedEnd]))
	{
		--originalEnd;
		--mappedEnd;
	}

	// Abort if we didn't manage to match anything.
	if (originalEnd == original.length())
		return;

	wxString originalUniquePart(original, originalEnd + 1);
	wxString mappedUniquePart(mapped, mappedEnd + 1);

	// Add the mapping if it's unique.
	for (std::vector<Mapping>::iterator i = inferMapping.begin();
			i != inferMapping.end(); ++i)
	{
		const Mapping &m = *i;
		if ((m.original == originalUniquePart) &&
			(m.mapped == mappedUniquePart))
			return;
	}

	Mapping m;
	m.original	= originalUniquePart;
	m.mapped	= mappedUniquePart;
	inferMapping.push_back(m);
}

bool Content::InferPath(SourceEditor *editor, const wxString &fileName)
{
	// Check if the filename starts with something we've matched before.
	for (std::vector<Mapping>::iterator i = inferMapping.begin();
			i != inferMapping.end(); ++i)
	{
		const Mapping &m = *i;
		if (fileName.StartsWith(m.original))
		{
			// Replace the start of the path with the mapped version.
			wxString inferredFileName = m.mapped;
			inferredFileName += fileName.Mid(m.original.length());

			// If the file exists then we use it.
			if (wxFileExists(inferredFileName) && editor->Load(inferredFileName))
				return true;
		}
	}

	return false;
}

void Content::iterateSourceEditors(std::function<void (SourceEditor&)> f)
{
	for (auto it = sourceEditors.begin(), endIt = sourceEditors.end(); it != endIt; ++it)
		f(*(it->second));
}

void Content::iterateSourceEditors(std::function<void (const SourceEditor&)> f) const
{
	for (auto it = sourceEditors.begin(), endIt = sourceEditors.end(); it != endIt; ++it)
		f(*(it->second));
}

void Content::RemoveSourceEditor(const wxString &fileName)
{
	auto it = sourceEditors.find(fileName);
	if (it != sourceEditors.end())
	{
		if (it->second == debugMarkedEditor)
			debugMarkedEditor = NULL;

		if (it->second == selectedEditor)
		{
			selectedEditor = NULL;
			mouseHover->StopHoverDetect();
		}

		sourceEditors.erase(it);
	}
}

void Content::OnPageClose(wxAuiNotebookEvent &event)
{
	SourceEditor *sourceEditor = static_cast<SourceEditor*>(GetPage(event.GetSelection()));
	if (sourceEditor)
		RemoveSourceEditor(sourceEditor->GetCurrentFile());
}

void Content::OnPageChanged(wxAuiNotebookEvent& event)
{
	int oldSelection = event.GetOldSelection();
	int newSelection = event.GetSelection();

	if (oldSelection == newSelection)
		return;

	if (newSelection != -1)
	{
		selectedEditor = (newSelection != -1 ? static_cast<SourceEditor*>(GetPage(newSelection)) : NULL);
		mouseHover->StartHoverDetect(selectedEditor);
	}
}

void Content::OnMouseHovering(wxCommandEvent &event)
{
	// TODO: Get the value from debugger and show in a ToolTip kind of window.
	//wxString word = selectedEditor->GetWordAtMouse();
	//host->GetWatchValue(0, word);
	//SetToolTip(word);
}
