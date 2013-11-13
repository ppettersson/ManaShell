#ifndef GDB_H
#define GDB_H

#include "../Debugger.h"
#include <queue>

class wxStringTokenizer;

class GDB : public Debugger
{
public:
	GDB(MainFrame *host);
	virtual ~GDB();


	// -- Run-time interface --------------------------------------------------

	virtual bool Attach();
	virtual bool Start();
	virtual void Stop();

	virtual void StepIn();
	virtual void StepOver();
	virtual void StepOut();

	virtual void Break();
	virtual void Continue();

	virtual void AddBreakpoint(const wxString &fileName, unsigned line);
	virtual void RemoveBreakpoint(const wxString &fileName, unsigned line);
	virtual void ClearAllBreakpoints();

	virtual void GetWatchValue(unsigned index, const wxString &variable);

	// Parse the command sent from the user to the debugger.
	virtual bool OnInterceptInput(const wxString &message);

	// Parse and possibly respond to the output from the debugger process.
	// Returning true means that the UI won't accept input.
	virtual bool OnOutput(const wxString &message);
	virtual bool OnError(const wxString &message);


	// -- User interface ------------------------------------------------------

	// The UI name for this plugin.
	virtual wxString GetName() const						{ return "C/C++ (GDB)"; }

	// Build up the full command from the current executable, script and
	// parameters with any extra glue that is necessary.
	virtual wxString GetCommand() const;


private:
	enum ExpectedOutput
	{
		kOutputUnknown,				// Uninitialized state, do nothing.

		kOutputBreakpoint,			//
		kOutputContinue,			//
		kOutputIdle,				// Waiting for input.
		kOutputNothing,				// No output except the prompt is expected.
		kOutputStart,				// Run to the first temporary breakpoint.
		kOutputStartup,				// Waiting for gdb to load symbols etc.
		kOutputStepping,			// Update the callstack interactively.
		kOutputSteppingOut,			// Returning from a frame.
		kOutputTemporaryBreakpoint,	// Controlled interrupt.
		kOutputUserBreak,			// Waiting for the program to be interrupted.
		kOutputUnexpected,			//
		kOutputQuitting				//
	};

	enum Command
	{
		kCommandNone,

		kCommandBreak,
		kCommandClear,
		kCommandDelete,
		kCommandContinue,
		kCommandFinish,
		kCommandNext,
		kCommandQuit,
		kCommandStart,				// Set a temporary breakpoint on the first line and run.
		kCommandStep
	};

	// Sometimes we have to string together a bunch of commands to be executed
	// in a sequence without any user input.
	struct CommandItem
	{
		wxString				message;
		Command					command;
		ExpectedOutput			expectedOutput;

		CommandItem()												{ }
		CommandItem(const wxString &m, Command c = kCommandNone, ExpectedOutput e = kOutputNothing)
			: message(m)
			, command(c)
			, expectedOutput(e)
		{
		}
	};

	// Access to the GUI and process handler.
	MainFrame					*host;

	// Information about the gdb process.
	long						majorVersion,
								minorVersion;
	bool						hasSymbols,
								programStarted;

	// All queued up commands that should be run whenever gdb is ready
	// for more input.
	std::queue<CommandItem>		commandQueue;

	// We parse thr output depending on the last command we sent to the
	// debugger.
	ExpectedOutput				expectedOutput;

	// This is used to track the current frame. It's used to detect when
	// we should request a full stack.
	wxString					currentFrame;

	// Remember the last command so we can better handle manual input.
	Command						lastCommand;


	bool ParseContinueOutput(wxStringTokenizer &lineTokenizer);
	bool ParseNothingOutput(wxStringTokenizer &lineTokenizer);
	bool ParseStartOutput(wxStringTokenizer &lineTokenizer);
	bool ParseStartupOutput(wxStringTokenizer &lineTokenizer);
	bool ParseSteppingOutput(wxStringTokenizer &lineTokenizer);
	bool ParseSteppingOutOutput(wxStringTokenizer &lineTokenizer);
	bool ParseSteppingOutError(wxStringTokenizer &lineTokenizer);
	bool ParseTemporaryBreakpointOutput(wxStringTokenizer &lineTokenizer);
	bool ParseUserBreakOutput(wxStringTokenizer &lineTokenizer);
	bool ParseUserBreakError(wxStringTokenizer &lineTokenizer);
	bool ParseUnexpectedOutput(wxStringTokenizer &lineTokenizer);
	bool ParseQuittingOutput(wxStringTokenizer &lineTokenizer);

	bool ParseDefaultError(const wxString &line);
	bool ParseDefaultOutput(const wxString &line);
	bool ParseFrame(const wxString &line, wxString &fileName, long &lineNr, wxString &frame);
	bool ParseSteppingWithinFrame(const wxString &line);

	bool RunCommandQueue();
};

#endif // GDB_H
