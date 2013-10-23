#include "../../GUI/Frames/Callstack.h"
#include "../../GUI/MainFrame.h"
#include "PDB.h"
#include "wx/tokenzr.h"

PDB::PDB(MainFrame *h)
	: Debugger(h)
	, host(h)
	, expectedOutput(kUnknown)
	, returningFromCall(false)
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
	expectedOutput = kStepping;	// kFullRefresh?
	return true;
}

void PDB::Stop()
{
	// Short hand for "quit".
	host->SendCommand("q\n");
	expectedOutput = kQuitting;
}

void PDB::StepIn()
{
	// Short hand for "step".
	host->SendCommand("s\n");
	expectedOutput = kStepping;
}

void PDB::StepOver()
{
	// Short hand for "next".
	host->SendCommand("n\n");
	expectedOutput = kStepping;
}

void PDB::StepOut()
{
	// Short hand for "return".
	host->SendCommand("r\n");
	expectedOutput = kStepping;	// ToDo: kFullRefresh?
}

void PDB::Break()
{
	// SIGINT?
	host->SendInterrupt();
	expectedOutput = kFullRefresh;
}

void PDB::Continue()
{
	// Short hand for "continue".
	host->SendCommand("c\n");
	expectedOutput = kFullRefresh;
}

void PDB::AddBreakpoint(const wxString &fileName, unsigned line)
{
	// b/break filename:line
	host->SendCommand(wxString::Format("b %s:%d\n", fileName, line));
	expectedOutput = kBreakpoint;
}

void PDB::RemoveBreakpoint(const wxString &fileName, unsigned line)
{
	// cl/clear filename:line
	host->SendCommand(wxString::Format("cl %s:%d\n", fileName, line));
	expectedOutput = kBreakpoint;
}

void PDB::ClearAllBreakpoints()
{
	// Clears all breaks. It normally asks for confirmation but that
	// should automatically be disabled when we're not in terminal mode.
	host->SendCommand("cl\n");
	expectedOutput = kBreakpoint;
}

void PDB::OnOutput(const wxString &message)
{
	// Break apart the message into lines.
	// Note that we match against any kind of line ending found on Linux, Mac and
	// Windows. On some systems that means that we get extra empty lines that we
	// have to deal with.
	wxStringTokenizer lineTokenizer(message, "\r\n");

	switch (expectedOutput)
	{
	case kBreakpoint:	ParseBreakpointOutput(lineTokenizer);	break;
	case kFullRefresh:	ParseFullRefreshOutput(lineTokenizer);	break;
	case kStepping:		ParseSteppingOutput(lineTokenizer);		break;
	}
}

void PDB::OnError(const wxString &message)
{
	if (expectedOutput == kQuitting)
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

void PDB::ParseBreakpointOutput(wxStringTokenizer &lineTokenizer)
{
	// ToDo: Should probably verify that we get the result we're already
	// assuming.
}

void PDB::ParseFullRefreshOutput(wxStringTokenizer &lineTokenizer)
{
	// ToDo: Get callstack.
}

void PDB::ParseSteppingOutput(wxStringTokenizer &lineTokenizer)
{
	while (lineTokenizer.HasMoreTokens())
	{
		wxString line = lineTokenizer.GetNextToken();

		// Check for some prefined strings that help us navigate the callstack.
		if (line == "--Call--")
		{
			// Push current stack frame.
			// We don't yet know the name of the new frame, but we reset
			// the current frame to force the regular source line pattern
			// matcher to push the new frame.
			currentFrame.Empty();
		}
		else if (line == "--Return--")
		{
			// Pop current stack frame.
			// Unfortunately this happens the next time we're stepping.
			// ToDo: figure out why...
			returningFromCall = true;
		}
		// We're expecting the output to be in this format:
		//	"> path/to/source.py(123)funcOrModule()" or
		//	"> path/to/source.py(123)funcOrModule()->None"
		//	when it's returning with a result.
		else if (line.Matches("> ?*(?*)?*()") ||	// fileName:lineNr:frame
				 line.Matches("> ?*(?*)?*()->?*"))	// fileName:lineNr:frame:result
		{
			// Read out the fileName after the prompt and before the first
			// paranthesis.
			wxString tail;
			wxString fileName = line.Mid(2).BeforeFirst('(', &tail);

			// Read out the line number between the paranthesises.
			long lineNr = 0;
			wxString tail2;
			tail.BeforeFirst(')', &tail2).ToLong(&lineNr);

			// Read out the frame before the paranthesises.
			wxString frame = tail2.BeforeFirst('(');

			host->UpdateSource(fileName, lineNr);

			// Update the callstack.
			if (frame != currentFrame)
			{
				currentFrame = frame;

				if (returningFromCall)
				{
					returningFromCall = false;
					PopStackFrame();

					// It's possible that we're jumping directly into a new
					// frame here if the last one was a constructor.
					if (currentFrame != host->GetCallstack()->CurrentFrame())
						PushStackFrame(frame, fileName, lineNr);
					else
						UpdateStackFrame(lineNr);
				}
				else
					PushStackFrame(frame, fileName, lineNr);
			}
			else
			{
				// Update the top most frame with the new line number.
				UpdateStackFrame(lineNr);
			}

			UpdateWatchedExpressions();
			return;
		}
		else
		{
			// It's probably output from the program. Try our luck with the
			// next line instead.
		}
	}

	// If we get here then we failed to parse out any line information.
	// For now we let it pass and hope for better luck the next time.
}

void PDB::PushStackFrame(const wxString &frame, const wxString &fileName, unsigned lineNr)
{
	host->GetCallstack()->PushFrame(frame, fileName, lineNr);
}

void PDB::PopStackFrame()
{
	host->GetCallstack()->PopFrame();
}

void PDB::UpdateStackFrame(unsigned lineNr)
{
	host->GetCallstack()->UpdateFrame(lineNr);
}

void PDB::UpdateWatchedExpressions()
{
}
