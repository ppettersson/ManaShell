#include "../MainFrame.h"
#include "Callstack.h"
#include "wx/textfile.h"
#include "wx/tokenzr.h"

BEGIN_EVENT_TABLE(Callstack, wxListBox)
	EVT_LISTBOX_DCLICK(wxID_ANY, Callstack::OnDClick)
END_EVENT_TABLE()



Callstack::Callstack(MainFrame *parent)
	: wxListBox(parent, wxID_ANY)
	, host(parent)
{
}

Callstack::~Callstack()
{
}

void Callstack::PushFrame(const wxString &description, const wxString &fileName, unsigned line)
{
	frames.push_back(Frame(description, fileName, line));

	Insert(description, 0);
	SetSelection(0);
}

void Callstack::PopFrame()
{
	Delete(0);
	SetSelection(0);

	frames.pop_back();
}

void Callstack::UpdateFrame(unsigned line)
{
	Frame &frame = frames.back();
	frame.line = line;
}

void Callstack::ClearAllFrames()
{
	Clear();
	frames.clear();
}

const wxString &Callstack::CurrentFrame() const
{
	const Frame &frame = frames.back();
	return frame.description;
}

const wxString &Callstack::PreviousFrame() const
{
	static wxString empty = wxEmptyString;
	if (frames.size() > 1)
		return frames[frames.size() - 2].description;
	else
		return empty;
}

bool Callstack::PopIfPreviousFrame(const wxString &frame)
{
	int numFrames = (int)frames.size();
	for (int i = numFrames - 1; i >= 0; --i)
		if (frame == frames[i].description)
		{
			for (int j = 0; j < numFrames - i - 1; ++j)
				Delete(0);
			SetSelection(0);

			frames.resize(i + 1);
			return true;
		}

	return false;
}

void Callstack::ResetToFrame(unsigned frame)
{
	unsigned numFrames = frames.size();
	if (frame < numFrames)
	{
		for (unsigned i = 0; i < numFrames - frame - 1; ++i)
			Delete(0);
		SetSelection(0);

		frames.resize(frame + 1);
	}
}

bool Callstack::Sync(const wxString &description, const wxString &fileName, unsigned lineNr)
{
	unsigned numFrames = frames.size();
	for (unsigned i = 0; i < numFrames; ++i)
		if (frames[i].description == description)
		{
			ResetToFrame(i);
			UpdateFrame(lineNr);
			return true;
		}

	return false;
}

bool Callstack::Load(const wxString &fileName)
{
	wxTextFile file;
	if (!file.Open(fileName))
	{
		wxMessageBox(wxString::Format("Failed to open file: %s", fileName), "Error");
		return false;
	}

	frames.clear();
	Clear();

	size_t numLines = file.GetLineCount();
	for (size_t i = numLines; i > 0; --i)
		LoadLine(file.GetLine(i - 1));

	return true;
}

void Callstack::LoadLine(const wxString &line)
{
	wxString	frameDescription,
				frameFileName;
	unsigned	frameLine = 0;

	wxStringTokenizer tokenizer(line, "|");
	if (tokenizer.HasMoreTokens())
	{
		frameDescription = tokenizer.GetNextToken();
		if (tokenizer.HasMoreTokens())
		{
			frameFileName = tokenizer.GetNextToken();
			if (tokenizer.HasMoreTokens())
			{
				long line = 0;
				if (tokenizer.GetNextToken().ToLong(&line) && line >= 0)
					frameLine = (unsigned)line;
			}
		}
	}

	PushFrame(frameDescription, frameFileName, frameLine);
}

void Callstack::OnDClick(wxCommandEvent &event)
{
	int selection = event.GetSelection();
	if (selection >= 0 && selection < (int)frames.size())
	{
		selection = frames.size() - selection - 1;

		const Frame &frame = frames[selection];
		host->UpdateSource(frame.line, frame.fileName);
	}
}
