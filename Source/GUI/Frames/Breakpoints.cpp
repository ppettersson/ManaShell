#include "../Dialogs/AddEditBreakpointDialog.h"
#include "../MainFrame.h"
#include "Breakpoints.h"
#include "wx/filename.h"

BEGIN_EVENT_TABLE(Breakpoints, wxListBox)
	EVT_LISTBOX_DCLICK(wxID_ANY, Breakpoints::OnDClick)
	EVT_CONTEXT_MENU(Breakpoints::OnContextMenu)
	EVT_MENU(kAddBreakpoint, Breakpoints::OnAddBreakpoint)
	EVT_MENU(kClearAllBreakpoints, Breakpoints::OnClearAllBreakpoints)
END_EVENT_TABLE()


Breakpoints::Breakpoints(MainFrame *parent)
	: wxListBox(parent, wxID_ANY)
	, host(parent)
{
}

Breakpoints::~Breakpoints()
{
}

Breakpoints::ToggleResult Breakpoints::ToggleBreak(const wxString &fileName, unsigned line)
{
	// Check if we already have a breakpoint at this position.
	bool exists = false;
	std::vector<Break>::iterator i;
	for (i = breaks.begin(); i != breaks.end(); ++i)
	{
		const Break &b = *i;
		if ((b.fileName == fileName) && (b.line == line))
		{
			exists = true;
			break;
		}
	}

	ToggleResult result;
	if (exists)
	{
		breaks.erase(i);
		result = kRemoved;
	}
	else
	{
		breaks.push_back(Break(fileName, line));
		result = kAdded;
	}

	UpdateUI();
	return result;
}

void Breakpoints::ClearAllBreakpoints()
{
	breaks.clear();
	UpdateUI();
}

void Breakpoints::GetLines(const wxString &fileName, std::vector<int> &out)
{
	for (std::vector<Break>::const_iterator it = breaks.begin(), endIt = breaks.end(); it != endIt; ++it)
	{
		if (it->fileName == fileName)
			out.push_back(it->line);
	}
}

bool Breakpoints::HasBreakpoint(const wxString &fileName, int line)
{
	for (std::vector<Break>::const_iterator it = breaks.begin(), endIt = breaks.end(); it != endIt; ++it)
	{
		if (it->fileName == fileName && (int)it->line == line)
			return true;
	}

	return false;
}

void Breakpoints::UpdateUI()
{
	Clear();
	for (std::vector<Break>::const_iterator i = breaks.begin(); i != breaks.end(); ++i)
	{
		// Don't show the full path.
		wxFileName fn;
		fn.Assign(i->fileName);

		Append(wxString::Format("%s:%d", fn.GetFullName(), i->line));
	}
}

void Breakpoints::OnDClick(wxCommandEvent &event)
{
	int selection = event.GetSelection();
	if (selection >= 0 && selection < (int)breaks.size())
	{
		const Break &b = breaks[selection];
		host->UpdateSource(b.line, b.fileName, false);
	}
}

void Breakpoints::OnContextMenu(wxContextMenuEvent &event)
{
	// Only allow changes when we're in interactive mode.
	if (!host->IsWaitingForResponse())
	{
		wxMenu *menu = new wxMenu;
		menu->Append(kAddBreakpoint, "&Add breakpoint");
		menu->Append(kClearAllBreakpoints, "&Clear all breakpoints");

		wxPoint position = event.GetPosition();

		// If the event was triggered from keyboard or other means then
		// ask for the mouse position directly instead.
		if (position == wxDefaultPosition)
			position = wxGetMousePosition();

		PopupMenu(menu, ScreenToClient(position));
	}
}

void Breakpoints::OnAddBreakpoint(wxCommandEvent &event)
{
	AddEditBreakpointDialog dialog(host);
	if (wxID_OK == dialog.ShowModal())
	{
		if (!HasBreakpoint(dialog.GetFileName(), dialog.GetLineNumber()))
			host->ToggleBreakpoint(dialog.GetFileName(), dialog.GetLineNumber());
	}
}

void Breakpoints::OnClearAllBreakpoints(wxCommandEvent &event)
{
	host->RequestClearAllBreakpoints();
}
