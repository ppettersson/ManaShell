#include "../../GUI/MainFrame.h"
#include "PDB.h"
#include "wx/tokenzr.h"

PDB::PDB(MainFrame *h)
	: Debugger(h)
	, host(h)
	, quitting(false)
{
	support.breakpoints	= true;
	support.callstack	= true;
	support.registers	= false;
	support.threads		= false;
	support.watch		= true;
	support.locals		= false;

#ifdef __WXMSW__
	executable			= "C:\\python33\\python.exe";
#else
	executable			= "python";
#endif
	script				= "script.py";
}

PDB::~PDB()
{
}

bool PDB::Attach()
{
	return false;
}

bool PDB::Start()
{
	return true;
}

void PDB::Stop()
{
	// Short hand for "quit".
	host->SendCommand("q\n");
	quitting = true;
}

void PDB::StepIn()
{
	// Short hand for "step".
	host->SendCommand("s\n");
}

void PDB::StepOver()
{
	// Short hand for "next".
	host->SendCommand("n\n");
}

void PDB::StepOut()
{
	// Short hand for "return".
	host->SendCommand("r\n");
}

void PDB::Break()
{
	// SIGINT?
}

void PDB::Continue()
{
	// Short hand for "continue".
	host->SendCommand("c\n");
}

void PDB::AddBreakpoint(const wxString &fileName, unsigned line)
{
	// b/break filename:line
	host->SendCommand(wxString::Format("b %s:%d\n", fileName, line));
}

void PDB::RemoveBreakpoint(const wxString &fileName, unsigned line)
{
	// cl/clear filename:line
	host->SendCommand(wxString::Format("cl %s:%d\n", fileName, line));
}

void PDB::ClearAllBreakpoints()
{
	// Clears all breaks. It normally asks for confirmation but that
	// should automatically be disabled when we're not in terminal mode.
	host->SendCommand("cl\n");
}

void PDB::OnOutput(const wxString &message)
{
	ParseUpdateSource(message);
}

void PDB::OnError(const wxString &message)
{
	if (quitting)
		host->SendCommand("quit()\n");
}

wxString PDB::GetCommand() const
{
	if (useCustomCommand)
		return customCommand;

	wxString result = executable;
	result += " -i -m pdb";
	if (!script.IsEmpty())
	{
		result += " ";
		result += script;
	}
	if (!arguments.IsEmpty())
	{
		result += " ";
		result += arguments;
	}
	return result;
}

void PDB::ParseUpdateSource(const wxString &message)
{
	// Break apart the message into lines.
	wxStringTokenizer lineTokenizer(message, "\n");
	while (lineTokenizer.HasMoreTokens())
	{
		// Parse out the source file and line number.
		wxString fileName;
		unsigned line = 0;

		// The general format is:
		//	"> path/to/source.py(123)<module>()"
		wxStringTokenizer tokenizer(lineTokenizer.GetNextToken(), " ()<");
		if (tokenizer.HasMoreTokens())
		{
			wxString token = tokenizer.GetNextToken();
			if (token == ">")
			{
				if (tokenizer.HasMoreTokens())
				{
					fileName = tokenizer.GetNextToken();
					if (tokenizer.HasMoreTokens())
					{
						long nr = 0;
						if (tokenizer.GetNextToken().ToLong(&nr) && nr >= 0)
							line = nr;
					}
				}
			}
		}

		if (!fileName.IsEmpty())
		{
			host->UpdateSource(fileName, line);
			return;
		}
	}
}
