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

	virtual void OnOutput(const wxString &message);
	virtual void OnError(const wxString &message);


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
		kFullRefresh,	// Completely lost, need to know everything.
		kUnknown,		// Uninitialized state, do nothing.
		kStepping,		// Update the callstack interactively.
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


	void ParseBreakpointOutput(wxStringTokenizer &lineTokenizer);
	void ParseFullRefreshOutput(wxStringTokenizer &lineTokenizer);
	void ParseSteppingOutput(wxStringTokenizer &lineTokenizer);

	void PushStackFrame(const wxString &frame, const wxString &fileName, unsigned lineNr);
	void PopStackFrame();
	void UpdateStackFrame(unsigned lineNr);

	void UpdateWatchedExpressions();
};

#endif // PDB_H
