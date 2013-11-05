#include "../../GUI/Frames/Callstack.h"
#include "../../GUI/Frames/Watch.h"
#include "../../GUI/MainFrame.h"
#include "PDB.h"
#include "wx/tokenzr.h"

PDB::PDB(MainFrame *h)
	: Debugger(h)
	, host(h)
	, expectedOutput(kUnknown)
	, startup(false)
	, returningFromCall(false)
	, getFullCallstack(false)
	, updateWatches(false)
	, currentWatch(0)
	, lastCommand(kNone)
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
	expectedOutput		= kStepping;
	getFullCallstack	= true;
	startup				= true;
	return true;
}

void PDB::Stop()
{
	// Short hand for "quit".
	host->SendCommand("q\n");
	lastCommand		= kQuit;
	expectedOutput	= kQuitting;
}

void PDB::StepIn()
{
	// Short hand for "step".
	host->SendCommand("s\n");
	lastCommand		= kStep;
	expectedOutput	= kStepping;
	updateWatches	= true;
}

void PDB::StepOver()
{
	// Short hand for "next".
	host->SendCommand("n\n");
	lastCommand		= kNext;
	expectedOutput	= kStepping;
	updateWatches	= true;
}

void PDB::StepOut()
{
	// Short hand for "return".
	host->SendCommand("r\n");
	lastCommand		= kReturn;
	expectedOutput	= kStepping;
	updateWatches	= true;
}

void PDB::Break()
{
	// SIGINT?
	host->SendInterrupt();
	lastCommand			= kBreak;
	expectedOutput		= kStepping;
	getFullCallstack	= true;
	updateWatches		= true;
}

void PDB::Continue()
{
	// Short hand for "continue".
	host->SendCommand("c\n");
	lastCommand			= kContinue;
	expectedOutput		= kStepping;
	getFullCallstack	= true;
	updateWatches		= true;
}

void PDB::AddBreakpoint(const wxString &fileName, unsigned line)
{
	wxASSERT(!fileName.IsEmpty());

	// b/break filename:line
	host->SendCommand(wxString::Format("b %s:%d\n", fileName, line));
	lastCommand		= kBreak;
	expectedOutput	= kBreakpoint;
}

void PDB::RemoveBreakpoint(const wxString &fileName, unsigned line)
{
	wxASSERT(!fileName.IsEmpty());

	// cl/clear filename:line
	host->SendCommand(wxString::Format("cl %s:%d\n", fileName, line));
	lastCommand		= kClear;
	expectedOutput	= kBreakpoint;
}

void PDB::ClearAllBreakpoints()
{
	// Clears all breaks. It normally asks for confirmation but that
	// should automatically be disabled when we're not in terminal mode.
	host->SendCommand("cl\n");
	lastCommand		= kClear;
	expectedOutput	= kBreakpoint;
}

void PDB::GetWatchValue(unsigned index, const wxString &variable)
{
	wxASSERT(!variable.IsEmpty());

	// A one off watch.
	host->SendCommand(wxString::Format("p %s\n", variable));
	lastCommand		= kPrint;
	expectedOutput	= kWatchOne;
	currentWatch	= index;
}

bool PDB::OnOutput(const wxString &message)
{
	// If we got this, then it should be running ok.
	startup = false;

	// Break apart the message into lines.
	// Note that we match against any kind of line ending found on Linux, Mac and
	// Windows. On some systems that means that we get extra empty lines that we
	// have to deal with.
	wxStringTokenizer lineTokenizer(message, "\r\n");

	switch (expectedOutput)
	{
	case kBreakpoint:
		ParseBreakpointOutput(lineTokenizer);
		return false;

	case kWatchOne:
		ParseWatchingOutput(lineTokenizer);
		return false;

	case kCallstack:	ParseCallstackOutput(lineTokenizer);	break;
	case kStepping:		ParseSteppingOutput(lineTokenizer);		break;
	case kWatching:		ParseWatchingOutput(lineTokenizer);		break;
	}

	if (getFullCallstack)
	{
		host->SendCommand("w\n");
		lastCommand			= kWhere;
		expectedOutput		= kCallstack;
		getFullCallstack	= false;
		return true;
	}

	if (updateWatches)
	{
		if (UpdateWatchedExpressions())
			return true;

		updateWatches = false;
	}

	return false;
}

bool PDB::OnError(const wxString &message)
{
	if (startup)
	{
		// Check for known causes first.
		wxString possibleCause;
		if (!useCustomCommand && !wxFileExists(script))
			possibleCause = wxString::Format("Failed to start the debugger, it seems like the script doesn't exist?\n\nScript = %s", script);
		else
			possibleCause = wxString::Format("Failed to start the debugger, is the command line correct?\n\nCommandLine = %s", GetCommand());

		wxMessageBox(possibleCause, "PDB Startup Error", wxOK | wxCENTRE | wxICON_ERROR, host);

		// Terminate the process.
		host->DebuggerTermination();
		host->SendCommand("quit()\n");
		lastCommand		= kQuit;
		expectedOutput	= kStartupError;
		startup			= false;
		return true;
	}

	if (expectedOutput == kQuitting)
		host->SendCommand("quit()\n");
	else
	{
		// Abort the current expect result.
		expectedOutput		= kUnknown;
		returningFromCall	= false;
		getFullCallstack	= false;
		updateWatches		= false;
	}

	return false;
}

wxString PDB::GetCommand() const
{
	if (useCustomCommand && !customCommand.IsEmpty())
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

void PDB::ParseCallstackOutput(wxStringTokenizer &lineTokenizer)
{
	wxString	fileName;
	long		lineNr;
	wxString	frame;

	host->GetCallstack()->ClearAllFrames();

	while (lineTokenizer.HasMoreTokens())
	{
		wxString line = lineTokenizer.GetNextToken();

		if (line.Matches("  ?*(?*)?*()") ||
			line.Matches("> ?*(?*)?*()"))
		{
			ParseFrame(line, fileName, lineNr, frame);
			host->GetCallstack()->PushFrame(frame, fileName, lineNr);
		}
		else if (line.Matches("-> ?*"))
		{
			// Ignore?
		}
		else
		{
			// Error!
		}
	}
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
			wxString	fileName;
			long		lineNr;
			wxString	frame;
			ParseFrame(line, fileName, lineNr, frame);

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

void PDB::ParseWatchingOutput(wxStringTokenizer &lineTokenizer)
{
	// For now we just take the output as it is and make no effort trying to
	// parse it.
	wxString value;

	while (lineTokenizer.HasMoreTokens())
	{
		wxString line = lineTokenizer.GetNextToken();

		if (line == "(Pdb) ")
		{
			// This marks the end.
		}
		else
		{
			if (!value.IsEmpty())
				value.Append("\n");
			value.Append(line);
		}
	}

	host->GetWatch()->Update(currentWatch, value, "");

	++currentWatch;
}

void PDB::ParseFrame(const wxString &line, wxString &fileName, long &lineNr, wxString &frame)
{
	// Read out the fileName after the prompt and before the first
	// paranthesis.
	wxString tail;
	fileName = line.Mid(2).BeforeFirst('(', &tail);

	// Read out the line number between the paranthesises.
	lineNr = 0;
	wxString tail2;
	tail.BeforeFirst(')', &tail2).ToLong(&lineNr);

	// Read out the frame before the paranthesises.
	frame = tail2.BeforeFirst('(');
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

bool PDB::UpdateWatchedExpressions()
{
	// Reset the index the first time.
	if (expectedOutput != kWatching)
		currentWatch = 0;

	// Get the next variable to watch.
	wxString variable;
	if (!host->GetWatch()->GetNext(currentWatch, variable))
		return false;

	wxASSERT(!variable.IsEmpty());
	host->SendCommand(wxString::Format("p %s\n", variable));
	lastCommand		= kPrint;
	expectedOutput	= kWatching;
	return true;
}
