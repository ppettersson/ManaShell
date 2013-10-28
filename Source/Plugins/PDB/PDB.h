#ifndef PDB_H
#define PDB_H

#include "../Debugger.h"

class wxStringTokenizer;

class PDB : public Debugger
{
public:
	PDB(MainFrame *host);
	virtual ~PDB();


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

	virtual bool OnOutput(const wxString &message);
	virtual bool OnError(const wxString &message);


	// -- User interface ------------------------------------------------------

	// The UI name for this plugin.
	virtual wxString GetName() const						{ return "Python"; }

	// Build up the full command from the current executable, script and
	// parameters with any extra glue that is necessary.
	virtual wxString GetCommand() const;


private:
	enum ExpectedOutput
	{
		kBreakpoint,	// Expecting an update about current breakpoints.
		kCallstack,		// Parse the full callstack.
		kUnknown,		// Uninitialized state, do nothing.
		kStepping,		// Update the callstack interactively.
		kWatching,		// Updating watched variables or expressions.
		kWatchOne,		// Update one specific variable or expression.
		kQuitting		// About to terminate the debugger.
	};


	// Access to the GUI and process handler.
	MainFrame		*host;

	// We parse the output depending on the last command we sent to the
	// debugger.
	ExpectedOutput	expectedOutput;

	// This is used to track the current stack frame. It's used to detect when
	// we should request a full stack.
	wxString		currentFrame;

	// This flag means that when the current stack frame changes then we should
	// expect it to be a pop instead of a push.
	bool			returningFromCall;

	// Immediately get the full callstack.
	bool			getFullCallstack;

	// Get the values for all watched variables or expressions.
	bool			updateWatches;

	// Current index of the watched variables or expressions.
	unsigned		currentWatch;


	void ParseBreakpointOutput(wxStringTokenizer &lineTokenizer);
	void ParseCallstackOutput(wxStringTokenizer &lineTokenizer);
	void ParseSteppingOutput(wxStringTokenizer &lineTokenizer);
	void ParseWatchingOutput(wxStringTokenizer &lineTokenizer);
	void ParseFrame(const wxString &line, wxString &fileName, long &lineNr, wxString &frame);

	void PushStackFrame(const wxString &frame, const wxString &fileName, unsigned lineNr);
	void PopStackFrame();
	void UpdateStackFrame(unsigned lineNr);

	bool UpdateWatchedExpressions();
};

#endif // PDB_H
