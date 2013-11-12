#include "../../GUI/Frames/Callstack.h"
#include "../../GUI/Frames/SourceEditor.h"
#include "../../GUI/Frames/Watch.h"
#include "../../GUI/MainFrame.h"
#include "GDB.h"
#include "wx/tokenzr.h"

GDB::GDB(MainFrame *h)
	: Debugger(h)
	, host(h)
	, majorVersion(0)
	, minorVersion(0)
	, hasSymbols(false)
	, programStarted(false)
	, expectedOutput(kOutputUnknown)
	, lastCommand(kCommandNone)
{
	support.breakpoints	= true;
	support.callstack	= true;
	support.registers	= false;	// ToDo
	support.threads		= false;	// ToDo
	support.watch		= true;
	support.locals		= false;	// ToDo

#ifdef __WXMSW__
	executable			= "C:\\Program Files (x86)\\CodeBlocks\\MinGW\\bin\\gdb.exe";
	script				= "E:\\tmp\\smallpt\\SmallPT_explicit\\bin\\Debug\\SmallPT_explicit.exe";	// "program.exe";
#else
	executable			= "gdb";
	script				= "program";
#endif
}

GDB::~GDB()
{
}

bool GDB::Attach()
{
	return false;
}

bool GDB::Start()
{
	expectedOutput = kOutputStartup;
	return true;
}

void GDB::Stop()
{
	//// Check if we have to interrupt the program first.
	//if (host->IsWaitingForResponse())
	//	host->SendInterrupt();
	//else
	//	// Short hand for "quit".
	//	host->SendCommand("quit\n");

	//lastCommand		= kCommandQuit;
	//expectedOutput	= kOutputQuitting;
}

void GDB::StepIn()
{
	commandQueue.push(CommandItem("step\n", kCommandStep, kOutputStepping));
	RunCommandQueue();
}

void GDB::StepOver()
{
	commandQueue.push(CommandItem("next\n", kCommandNext, kOutputStepping));
	RunCommandQueue();
}

void GDB::StepOut()
{
	commandQueue.push(CommandItem("finish\n", kCommandFinish, kOutputSteppingOut));
	RunCommandQueue();
}

void GDB::Break()
{
	//host->SendInterrupt();
	//expectedOutput	= kOutputUnexpected;
}

void GDB::Continue()
{
	//host->GetSourceEditor()->DisableDebugMarker();
	//host->SendCommand("continue\n");
	//lastCommand		= kCommandContinue;
	//expectedOutput	= kOutputUnexpected;
}

void GDB::AddBreakpoint(const wxString &fileName, unsigned line)
{
	//wxASSERT(!fileName.IsEmpty());

	//host->SendCommand(wxString::Format("break %s:%d\n", fileName, line));
	//lastCommand		= kCommandBreak;
	//expectedOutput	= kOutputBreakpoint;
}

void GDB::RemoveBreakpoint(const wxString &fileName, unsigned line)
{
	//wxASSERT(!fileName.IsEmpty());

	//host->SendCommand(wxString::Format("clear %s:%d\n", fileName, line));
	//lastCommand		= kCommandClear;
	//expectedOutput	= kOutputBreakpoint;
}

void GDB::ClearAllBreakpoints()
{
	// Clears all breaks. It normally asks for confirmation but that
	// should automatically be disabled when we're not in terminal mode.
	//host->SendCommand("delete\n");
	//lastCommand		= kCommandDelete;
	//expectedOutput	= kOutputBreakpoint;
}

void GDB::GetWatchValue(unsigned index, const wxString &variable)
{
}

// Parse the command sent from the user to the debugger.
bool GDB::OnInterceptInput(const wxString &message)
{
	return false;
}

// Parse and possibly respond to the output from the debugger process.
// Returning true means that the UI won't accept input.
bool GDB::OnOutput(const wxString &message)
{
	// Break apart the message into lines.
	// Note that we match against any kind of line ending found on Linux, Mac and
	// Windows. On some systems that means that we get extra empty lines that we
	// have to deal with.
	wxStringTokenizer lineTokenizer(message, "\r\n");

	switch (expectedOutput)
	{
	case kOutputIdle:					return false;
	case kOutputStart:					return ParseStartOutput(lineTokenizer);
	case kOutputStartup:				return ParseStartupOutput(lineTokenizer);
	case kOutputStepping:				return ParseSteppingOutput(lineTokenizer);
	case kOutputSteppingOut:			return ParseSteppingOutOutput(lineTokenizer);
	case kOutputTemporaryBreakpoint:	return ParseTemporaryBreakpointOutput(lineTokenizer);
	case kOutputUnexpected:				return ParseUnexpectedOutput(lineTokenizer);
	case kOutputQuitting:				return ParseQuittingOutput(lineTokenizer);
	case kOutputNothing:				return ParseNothingOutput(lineTokenizer);
	}

	return true;
}

bool GDB::OnError(const wxString &message)
{
	wxStringTokenizer lineTokenizer(message, "\r\n");

	switch (expectedOutput)
	{
	case kOutputIdle:					return false;
	case kOutputSteppingOut:			return ParseSteppingOutError(lineTokenizer);
	}

	return true;
}

// Build up the full command from the current executable, script and
// parameters with any extra glue that is necessary.
wxString GDB::GetCommand() const
{
	if (useCustomCommand)
		return customCommand;

	wxString result = executable;
	if (!script.IsEmpty() || !arguments.IsEmpty())
	{
		result += " -silent --args ";
		if (!script.IsEmpty())
			result += script;
		if (!arguments.IsEmpty())
		{
			result += " ";
			result += arguments;
		}
	}
	return result;
}

bool GDB::ParseStartOutput(wxStringTokenizer &lineTokenizer)
{
	while (lineTokenizer.HasMoreTokens())
	{
		wxString line = lineTokenizer.GetNextToken();

		// "Temporary breakpoint 1 at 0x40290e: file c:\code\program.cpp, line 123"
		if (line.Matches("Temporary breakpoint ?* at ?*: file ?*, line ?*"))
		{
			expectedOutput = kOutputTemporaryBreakpoint;
		}
		// "Starting program: c:\code\program.exe"
		else if (line.Matches("Starting program: ?*"))
		{
			// gdb should have set a temporary breakpoint by now.
			wxASSERT(expectedOutput == kOutputTemporaryBreakpoint);

			programStarted = true;

			if (lineTokenizer.HasMoreTokens())
				return ParseTemporaryBreakpointOutput(lineTokenizer);

			return true;
		}
		else
			ParseDefaultOutput(line);
	}

	return true;
}

bool GDB::ParseStartupOutput(wxStringTokenizer &lineTokenizer)
{
	// See if we got any errors that we recognize.
	// Then search for the command prompt.
	while (lineTokenizer.HasMoreTokens())
	{
		wxString line = lineTokenizer.GetNextToken();

		// Check if we got symbols.
		if (line.Matches("Reading symbols from ?*"))
		{
			hasSymbols = true;
		}
		// Get the version number.
		else if (line.Matches("GNU gdb (GDB) ?*"))
		{
			wxString version = line.AfterLast(' ');

			version.BeforeFirst('.').ToLong(&majorVersion);
			version.AfterLast('.').ToLong(&minorVersion);

			// ToDo: figure out what we're compatible with and if we get
			// anything extra nice that we can use.
		}
		// The prompt marks the end.
		else if (line == "(gdb) ")
		{
			// Verify that we don't have any more lines.
			if (!lineTokenizer.HasMoreTokens())
			{
				// Issue a warning if we didn't manage to load any symbols.
				if (!hasSymbols)
					wxMessageBox("Couldn't read any symbols", "Warning", wxOK | wxCENTRE | wxICON_WARNING, host);

				// Concatenate some settings where we don't expect any output
				// or error handling.
				// Disable text wrapping and pagination to simplify parsing.
				commandQueue.push(CommandItem("set width 0\n"));
				commandQueue.push(CommandItem("set height 0\n"));

				// We're ready.
				// Set a temporary breakpoint in main and continue.
				commandQueue.push(CommandItem("start\n", kCommandStart, kOutputStart));
				return RunCommandQueue();
			}
			else
			{
				// Continue to parse the lines and hope we get another prompt.
			}
		}
		else
		{
			// There's a bunch of startup information about copyright etc that
			// has to be skipped.
		}
	}

	return true;
}

bool GDB::ParseNothingOutput(wxStringTokenizer &lineTokenizer)
{
	// We expect a prompt here with no more tokens.
	while (lineTokenizer.HasMoreTokens())
	{
		wxString line = lineTokenizer.GetNextToken();

		if (line.Matches("(gdb) "))
		{
			if (!lineTokenizer.HasMoreTokens())
			{
				return RunCommandQueue();
			}
			else
			{
				// Continue to parse the lines and hope we get another prompt.
			}
		}
		else
			ParseDefaultOutput(line);
	}

	return true;
}

bool GDB::ParseSteppingOutput(wxStringTokenizer &lineTokenizer)
{
	while (lineTokenizer.HasMoreTokens())
	{
		wxString line = lineTokenizer.GetNextToken();

		// Stepping within a frame.
		if (ParseSteppingWithinFrame(line))
			;
		// Stepping to a new frame.
		else if (line.Matches("?* at ?*:?*"))
		{
			wxString	fileName,
						frame;
			long		lineNr = 0;
			ParseFrame(line, fileName, lineNr, frame);

			if (frame != currentFrame)
			{
				// ToDo: Same problem as with PDB, there's no information about
				// stepping from an outer frame directly to another outer
				// frame... Empirically it seems to work best to just search
				// for a matching frame.

				// Check if it's a step up or down.
				Callstack *callstack = host->GetCallstack();
				if (!callstack->PopIfPreviousFrame(frame))
					callstack->PushFrame(frame, fileName, lineNr);

				currentFrame = frame;
			}
		}
		else if (line.Matches("(gdb) "))
		{
			// Ready to accept input now.
			return false;
		}
		else
			ParseDefaultOutput(line);
	}

	return true;
}

bool GDB::ParseSteppingOutOutput(wxStringTokenizer &lineTokenizer)
{
	Callstack *callstack = host->GetCallstack();

	while (lineTokenizer.HasMoreTokens())
	{
		wxString line = lineTokenizer.GetNextToken();

		// Run till exit from #0  func (r=..., id=@0x28f514: 0) at path/to/file.cpp:45
		if (line.Matches("Run till exit from #?* ?* at ?*:?*"))
		{
			// Go back one step in the callstack.
			callstack->PopFrame();
			currentFrame = callstack->CurrentFrame();
		}
		// 0x004013bc in func (r=..., E=1) at path/to/file.cpp:9
		else if (line.Matches("?* in ?* at ?*:?*"))
		{
			// Sync up which frame we're actually in.

			// Search for the split markers in the string.
			size_t frameStart = line.Find(" in ");

			wxString	frame,
						fileName;
			long		lineNr = 0;
			ParseFrame(line.Mid(frameStart + 4), fileName, lineNr, frame);

			callstack->Sync(frame, fileName, lineNr);
		}
		// Stepping within a frame.
		else if (ParseSteppingWithinFrame(line))
			;
		// Value returned is $1 = true
		else if (line.Matches("Value returned is $?* = ?*"))
		{
			// ToDo?
		}
		else if (line == "(gdb) ")
		{
			// Ready to accept input now.
			return false;
		}
		else
			ParseDefaultOutput(line);
	}

	return true;
}

bool GDB::ParseSteppingOutError(wxStringTokenizer &lineTokenizer)
{
	while (lineTokenizer.HasMoreTokens())
	{
		wxString line = lineTokenizer.GetNextToken();

		// Handle error when trying to step out of the bottom frame.
		if (line == "\"finish\" not meaningful in the outermost frame.")
		{
			// It turns into a no-op and we fall back to regular stepping.
			expectedOutput = kOutputStepping;
		}
		else
			ParseDefaultError(line);
	}

	return true;
}

bool GDB::ParseTemporaryBreakpointOutput(wxStringTokenizer &lineTokenizer)
{
	while (lineTokenizer.HasMoreTokens())
	{
		wxString line = lineTokenizer.GetNextToken();

		// "Temporary breakpoint 1, main (argc=1, argv=0x12341234) at c:\code\program.cpp:123"
		if (line.Matches("Temporary breakpoint ?*, ?* at ?*:?*"))
		{
			// Search for the split markers in the string.
			size_t frameStart = line.Find(", ");

			wxString	frame,
						fileName;
			long		lineNr = 0;
			ParseFrame(line.Mid(frameStart + 2), fileName, lineNr, frame);

			host->UpdateSource(lineNr, fileName);

			// Update the callstack.
			if (frame != currentFrame)
			{
				currentFrame = frame;
				host->GetCallstack()->PushFrame(frame, fileName, lineNr);
			}

			expectedOutput = kOutputStepping;
			if (lineTokenizer.HasMoreTokens())
				return ParseSteppingOutput(lineTokenizer);
		}
		else
			ParseDefaultOutput(line);
	}

	return true;
}

bool GDB::ParseUnexpectedOutput(wxStringTokenizer &lineTokenizer)
{
	return true;
}

bool GDB::ParseQuittingOutput(wxStringTokenizer &lineTokenizer)
{
	return true;
}

void GDB::ParseDefaultError(const wxString &line)
{
	// Unknown error.
	wxASSERT(false);
}

void GDB::ParseDefaultOutput(const wxString &line)
{
	if (line.Matches("[New Thread ?*]"))
	{
		// ToDo?
	}
	else if (line.Matches("(gdb) "))
	{
		// We got a prompt where we didn't expect it.
		wxASSERT(false);
	}
	else
	{
		// ToDo: program output?
		wxASSERT(false);
	}
}

void GDB::ParseFrame(const wxString &line, wxString &fileName, long &lineNr, wxString &frame)
{
	size_t frameEnd = line.Find(" at ");
	size_t lineStart = line.find_last_of(':');
	if ((frameEnd == wxNOT_FOUND) ||
		(lineStart == wxNOT_FOUND))
		return;

	// Extract the frame and fileName.
	frame = line.SubString(0, frameEnd - 1);
	fileName = line.SubString(frameEnd + 4, lineStart - 1);

	// Parse out the line number.
	line.AfterLast(':').ToLong(&lineNr);
}

bool GDB::ParseSteppingWithinFrame(const wxString &line)
{
	if (line.Matches("?*\t?*"))
	{
		long lineNr = -1;
		line.BeforeFirst('\t').ToLong(&lineNr);

		if (lineNr >= 0)
			host->UpdateSource(lineNr);

		return true;
	}

	return false;
}

bool GDB::RunCommandQueue()
{
	// Check if there are any commands waiting.
	if (!commandQueue.empty())
	{
		// First In First Out.
		const CommandItem &c = commandQueue.front();

		// Send off the command and rig the state machine.
		host->SendCommand(c.message);
		lastCommand		= c.command;
		expectedOutput	= c.expectedOutput;

		commandQueue.pop();
		return true;
	}

	return false;
}
