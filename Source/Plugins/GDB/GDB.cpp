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
	executable			= "C:\\Program Files (x86)\\CodeBlocks\\MinGW\\bin\\gdb.exe";				// "C:\\Program Files (x86)\\MinGW\\bin\\gdb.exe";
	script				= "E:\\tmp\\smallpt\\SmallPT_explicit\\bin\\Debug\\SmallPT_explicit.exe";	// "program.exe";
#else
	executable			= "gdb";
	script				= "program";
#endif
}

GDB::~GDB()
{
}

bool GDB::Start()
{
	expectedOutput = kOutputStartup;
	return true;
}

void GDB::Stop()
{
	// Check if we have to interrupt the program first.
	//wxASSERT(!host->IsWaitingForResponse());
	//if (programStarted && host->IsWaitingForResponse())
	//{
	//	host->SendInterrupt();
	//	// ToDo: empty queue.
	//}
	//else
	{
		while (!commandQueue.empty())
			commandQueue.pop();

		commandQueue.push(CommandItem("quit\n", kCommandQuit, kOutputQuitting));
		RunCommandQueue();
	}
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
	host->SendInterrupt();
	expectedOutput = kOutputUserBreak;
}

void GDB::Continue()
{
	host->GetSourceEditor()->DisableDebugMarker();
	commandQueue.push(CommandItem("continue\n", kCommandContinue, kOutputContinue));
	RunCommandQueue();
}

void GDB::AddBreakpoint(const wxString &fileName, unsigned line)
{
	wxASSERT(!fileName.IsEmpty());

	commandQueue.push(CommandItem(wxString::Format("break %s:%d\n", fileName, line), kCommandBreak, kOutputBreakpoint));
	RunCommandQueue();
}

void GDB::RemoveBreakpoint(const wxString &fileName, unsigned line)
{
	wxASSERT(!fileName.IsEmpty());

	commandQueue.push(CommandItem(wxString::Format("clear %s:%d\n", fileName, line), kCommandClear, kOutputBreakpoint));
	RunCommandQueue();
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
	case kOutputBreakpoint:				return ParseBreakpointOutput(lineTokenizer);
	case kOutputContinue:				return ParseContinueOutput(lineTokenizer);
	case kOutputIdle:					return false;
	case kOutputStart:					return ParseStartOutput(lineTokenizer);
	case kOutputStartup:				return ParseStartupOutput(lineTokenizer);
	case kOutputStepping:				return ParseSteppingOutput(lineTokenizer);
	case kOutputSteppingOut:			return ParseSteppingOutOutput(lineTokenizer);
	case kOutputTemporaryBreakpoint:	return ParseTemporaryBreakpointOutput(lineTokenizer);
	case kOutputUserBreak:				return ParseUserBreakOutput(lineTokenizer);
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
	//case kOutputBreakpoint:				return ParseDefaultError(lineTokenizer);
	//case kOutputContinue:				return ParseDefaultError(lineTokenizer);
	case kOutputIdle:					return false;
	case kOutputStart:					return ParseStartError(lineTokenizer);
	//case kOutputStartup:				return ParseDefaultError(lineTokenizer);
	//case kOutputStepping:				return ParseDefaultError(lineTokenizer);
	case kOutputSteppingOut:			return ParseSteppingOutError(lineTokenizer);
	//case kOutputTemporaryBreakpoint:	return ParseDefaultError(lineTokenizer);
	case kOutputUserBreak:				return ParseUserBreakError(lineTokenizer);
	//case kOutputUnexpected:				return ParseDefaultError(lineTokenizer);
	//case kOutputQuitting:				return ParseDefaultError(lineTokenizer);
	//case kOutputNothing:				return ParseDefaultError(lineTokenizer);
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

bool GDB::ParseBreakpointError(wxStringTokenizer &lineTokenizer)
{
	return true;
}

bool GDB::ParseBreakpointOutput(wxStringTokenizer &lineTokenizer)
{
	bool success = false;

	while (lineTokenizer.HasMoreTokens())
	{
		wxString line = lineTokenizer.GetNextToken();

		if (line.Matches("Breakpoint ?* at ?*: file ?*, line ?*."))
		{
			success = true;
		}
		else if (line == "(gdb) ")
		{
			// Return user input.
			return false;
		}
	}

	return true;
}

bool GDB::ParseContinueOutput(wxStringTokenizer &lineTokenizer)
{
	while (lineTokenizer.HasMoreTokens())
	{
		wxString line = lineTokenizer.GetNextToken();

		if (line == "Continuing.")
		{
			expectedOutput = kOutputUnexpected;
			return ParseUnexpectedOutput(lineTokenizer);
		}
		else
			ParseDefaultOutput(line);
	}

	return true;
}

bool GDB::ParseStartError(wxStringTokenizer &lineTokenizer)
{
	while (lineTokenizer.HasMoreTokens())
	{
		wxString line = lineTokenizer.GetNextToken();

		if (line == "The \"remote\" target does not support \"run\".  "
					"Try \"help target\" or \"continue\".")
		{
			// This means that we're dealing with remote debugging.
			// Go to interactive mode and let the user decide.
			programStarted = true;
		}
		else
			ParseDefaultError(line);
	}

	return true;
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
		else if (line == "(gdb) ")
		{
			// If we're remote debugging then we got an error message but
			// handled it and should enter interactive mode.
			if (programStarted)
				return false;

			// Warn about this, but give the user the possibility to debug
			// manually.
			wxMessageBox("Failed to start the program and/or attach the debugger.",
						 "Warning", wxOK | wxCENTRE | wxICON_ERROR, host);
			expectedOutput = kOutputIdle;
			return false;
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
		if (line.Matches("Reading symbols from ?*") ||
			(line == "Attaching and reading symbols, this may take a while.."))
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
			//wxASSERT(!lineTokenizer.HasMoreTokens());

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
			//wxASSERT(!lineTokenizer.HasMoreTokens());
			return RunCommandQueue();
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
			//wxASSERT(!lineTokenizer.HasMoreTokens());
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
			//wxASSERT(lineTokenizer.HasMoreTokens());
			return false;
		}
		else
			ParseDefaultOutput(line);
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

bool GDB::ParseUserBreakError(wxStringTokenizer &lineTokenizer)
{
	while (lineTokenizer.HasMoreTokens())
	{
		wxString line = lineTokenizer.GetNextToken();

		if (ParseDefaultError(line))
			return true;
	}

	return true;
}

bool GDB::ParseUserBreakOutput(wxStringTokenizer &lineTokenizer)
{
	while (lineTokenizer.HasMoreTokens())
	{
		wxString line = lineTokenizer.GetNextToken();

		if (line == "Program received signal SIGINT, Interrupt.")
		{
			expectedOutput = kOutputStepping;
		}
		else if (line == "(gdb) ")
		{
			// We're ready for user input.
			//wxASSERT(!lineTokenizer.HasMoreTokens());
			return false;
		}
		else
			ParseDefaultOutput(line);
	}

	return true;
}

bool GDB::ParseUnexpectedOutput(wxStringTokenizer &lineTokenizer)
{
	while (lineTokenizer.HasMoreTokens())
	{
		wxString line = lineTokenizer.GetNextToken();

		if (line.Matches("[Inferior ?* (process ?*) exited normally]"))
		{
			// The program exited and can't be debugged any more.
			programStarted = false;
			Stop();
			return true;
		}
		else if (line.Matches("Breakpoint ?*, ?* at ?*:?*"))
		{
			// Sync up which frame we're actually in.

			// Search for the split markers in the string.
			size_t frameStart = line.Find(", ");

			wxString	frame,
						fileName;
			long		lineNr = 0;
			ParseFrame(line.Mid(frameStart + 2), fileName, lineNr, frame);

			Callstack *callstack = host->GetCallstack();
			if (callstack->GetNumFrames() > 0)
				callstack->Sync(frame, fileName, lineNr);
			else
			{
				// ToDo: Request the full callstack.
				callstack->PushFrame(frame, fileName, lineNr);
			}
			host->UpdateSource(lineNr, fileName);

			// Program interrupted due to breakpoint.
			expectedOutput = kOutputStepping;
			return ParseSteppingOutput(lineTokenizer);
		}
		else
			ParseDefaultOutput(line);
	}

	return true;
}

bool GDB::ParseQuittingOutput(wxStringTokenizer &lineTokenizer)
{
	while (lineTokenizer.HasMoreTokens())
	{
		wxString line = lineTokenizer.GetNextToken();

		if (line == "Quit anyway? (y or n) [answered Y; input not from terminal]")
		{
			// This is the only line we care about, return control to the user.
			return false;
		}
	}

	return true;
}

bool GDB::ParseDefaultError(const wxString &line)
{
	if (line == "Quit (expect signal SIGINT when the program is resumed)")
	{
		// The program had time to quit before we could finish the current
		// operation, it can no longer be debugged.
		programStarted = false;
		Stop();
		return true;
	}

	// Unknown error.
	//wxASSERT(false);
	return false;
}

bool GDB::ParseDefaultOutput(const wxString &line)
{
	if (line.Matches("[New Thread ?*]") ||
		line.Matches("[Switching to Thread ?*]") ||
		line.Matches("?* in ?* () from ?*"))
	{
		// ToDo: Threads
		return true;
	}
	else if (line.Matches("$?* = ?*"))
	{
		// Ignore.
	}
	else if (line == "(gdb) ")
	{
		// We got a prompt where we didn't expect it.
		wxASSERT(false);
	}
	else
	{
		// ToDo: program output?
		//wxASSERT(false);
	}

	// ToDo:
	// "0x12341234 in KERNELBASE!DebugBreak () from c:\windows\kernelbase.dll"
	return false;
}

bool GDB::ParseFrame(const wxString &line, wxString &fileName, long &lineNr, wxString &frame)
{
	size_t frameEnd = line.Find(" at ");
	size_t lineStart = line.find_last_of(':');
	if ((frameEnd == wxNOT_FOUND) ||
		(lineStart == wxNOT_FOUND))
		return false;

	// Extract the frame and fileName.
	frame = line.SubString(0, frameEnd - 1);
	fileName = line.SubString(frameEnd + 4, lineStart - 1);

	// Parse out the line number.
	line.AfterLast(':').ToLong(&lineNr);
	return true;
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
