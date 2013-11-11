#include "../../GUI/Frames/Callstack.h"
#include "../../GUI/Frames/SourceEditor.h"
#include "../../GUI/Frames/Watch.h"
#include "../../GUI/MainFrame.h"
#include "GDB.h"
#include "wx/tokenzr.h"

GDB::GDB(MainFrame *h)
	: Debugger(h)
	, host(h)
	, expectedOutput(kUnknown)
	, lastCommand(kNone)
{
	support.breakpoints	= true;
	support.callstack	= true;
	support.registers	= false;	// ToDo
	support.threads		= false;	// ToDo
	support.watch		= true;
	support.locals		= false;	// ToDo

#ifdef __WXMSW__
	executable			= "C:\\Program Files (x86)\\CodeBlocks\\MinGW\\bin\\gdb.exe";
#else
	executable			= "gdb";
#endif
	//script				= "program.exe";
	script				= "E:\\tmp\\smallpt\\SmallPT_explicit\\bin\\Debug\\SmallPT_explicit.exe";
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
	expectedOutput = kStartup;
	return true;
}

void GDB::Stop()
{
	// ToDo
}

void GDB::StepIn()
{
	host->SendCommand("s\n");
	lastCommand		= kStep;
	expectedOutput	= kStepping;
}

void GDB::StepOver()
{
	host->SendCommand("n\n");
	lastCommand		= kNext;
	expectedOutput	= kStepping;
}

void GDB::StepOut()
{
	host->SendCommand("fin\n");
	lastCommand		= kFinish;
	expectedOutput	= kSteppingOut;
}

void GDB::Break()
{
	// ToDo
}

void GDB::Continue()
{
	host->SendCommand("c\n");
	lastCommand		= kContinue;
	expectedOutput	= kUnexpected;
}

void GDB::AddBreakpoint(const wxString &fileName, unsigned line)
{
}

void GDB::RemoveBreakpoint(const wxString &fileName, unsigned line)
{
}

void GDB::ClearAllBreakpoints()
{
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
	case kStartup:
		ParseStartupOutput(lineTokenizer);
		return true;

	case kStepping:
		ParseSteppingOutput(lineTokenizer);
		break;

	case kSteppingOut:
		ParseSteppingOutOutput(lineTokenizer);
		break;

	case kTemporaryBreakpoint:
		ParseTemporaryBreakpointOutput(lineTokenizer);
		break;
	}

	// It's ok to enable user input, we're not expecting any more output at the
	// moment.
	return false;
}

bool GDB::OnError(const wxString &message)
{
	wxStringTokenizer lineTokenizer(message, "\r\n");

	switch (expectedOutput)
	{
	case kSteppingOut:
		ParseSteppingOutError(lineTokenizer);
		break;
	}

	return false;
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
		result += " --args ";
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

void GDB::ParseStartupOutput(wxStringTokenizer &lineTokenizer)
{
	// See if we got any errors that we recognize.
	// Then search for the command prompt.

	long	majorVersion = 0,
			minorVersion = 0;

	while (lineTokenizer.HasMoreTokens())
	{
		wxString line = lineTokenizer.GetNextToken();

		// Get the version number.
		if (line.Matches("GNU gdb (GDB) ?*"))
		{
			wxString version = line.AfterLast(' ');

			version.BeforeFirst('.').ToLong(&majorVersion);
			version.AfterLast('.').ToLong(&minorVersion);

			// ToDo: figure out what we're compatible with and if we get
			// anything extra nice that we can use.
		}
		else if (line == "(gdb) ")
		{
			// Verify that we don't have any more lines.
			if (!lineTokenizer.HasMoreTokens())
			{
				// ToDo: disable text wrapping and pagination:
				// "set width 0"
				// "set height 0"

				// We're ready.
				// Set a temporary breakpoint in main and continue.
				//host->SendCommand("start\n");
				host->SendCommand("set width 0\nset height 0\nstart\n");
				expectedOutput	= kTemporaryBreakpoint;
				lastCommand		= kStart;
				return;
			}
			else
			{
				// Continue to parse the lines and hope we get another prompt.
			}
		}
	}
}

void GDB::ParseSteppingOutput(wxStringTokenizer &lineTokenizer)
{
	while (lineTokenizer.HasMoreTokens())
	{
		wxString line = lineTokenizer.GetNextToken();

		// Stepping within a frame.
		if (line.Matches("?*\t?*"))
		{
			long lineNr = -1;
			line.BeforeFirst('\t').ToLong(&lineNr);

			if (lineNr >= 0)
				host->UpdateSource(lineNr);
		}
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
	}
}

void GDB::ParseSteppingOutOutput(wxStringTokenizer &lineTokenizer)
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
		// 123\t  if (!intersect(r, id))
		else if (line.Matches("?*\t?*"))
		{
			long lineNr = -1;
			line.BeforeFirst('\t').ToLong(&lineNr);

			if (lineNr >= 0)
				host->UpdateSource(lineNr);
		}
		// Value returned is $1 = true
		else if (line.Matches("Value returned is $?* = ?*"))
		{
			// ToDo?
		}
		else if (line == "(gdb) ")
		{
			// We're done.
		}
		else
		{
			// Error?
		}
	}
}

void GDB::ParseSteppingOutError(wxStringTokenizer &lineTokenizer)
{
	while (lineTokenizer.HasMoreTokens())
	{
		wxString line = lineTokenizer.GetNextToken();

		// Handle error when trying to step out of the bottom frame.
		if (line == "\"finish\" not meaningful in the outermost frame.")
		{
			// ToDo?
		}
	}
}

void GDB::ParseTemporaryBreakpointOutput(wxStringTokenizer &lineTokenizer)
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

			expectedOutput = kStepping;
		}
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
