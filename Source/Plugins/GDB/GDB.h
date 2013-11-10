#ifndef GDB_H
#define GDB_H

#include "../Debugger.h"

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
		kUnknown,				// Uninitialized state, do nothing.

		kStartup,				// Waiting for gdb to load symbols etc.
		kStepping,				// Update the callstack interactively.
		kUnexpected,
		kTemporaryBreakpoint	// Controlled interrupt.
	};

	enum Command
	{
		kNone,

		kStart,					// Set a temporary breakpoint on the first line and run.
		kStep,
		kNext,
		kFinish,
		kContinue
	};

	// Access to the GUI and process handler.
	MainFrame		*host;

	// We parse thr output depending on the last command we sent to the
	// debugger.
	ExpectedOutput	expectedOutput;

	// This is used to track the current frame. It's used to detect when
	// we should request a full stack.
	wxString		currentFrame;

	// Remember the last command so we can better handle manual input.
	Command			lastCommand;


	void ParseStartupOutput(wxStringTokenizer &lineTokenizer);
	void ParseSteppingOutput(wxStringTokenizer &lineTokenizer);
	void ParseTemporaryBreakpointOutput(wxStringTokenizer &lineTokenizer);

	void ParseFrame(const wxString &line, wxString &fileName, long &lineNr, wxString &frame);
};

#endif // GDB_H
