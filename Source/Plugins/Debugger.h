#ifndef DEBUGGER_H
#define DEBUGGER_H

#include "wx/wx.h"

class MainFrame;

class Debugger
{
public:
	struct Support
	{
		unsigned	breakpoints	: 1,
					callstack	: 1,
					registers	: 1,
					threads		: 1,
					watch		: 1,
					locals		: 1;
	};


	static Debugger *Create(MainFrame *host);

	Debugger(MainFrame *host) : useCustomCommand(false)		{ }
	virtual ~Debugger()										{ }


	// -- Run-time interface --------------------------------------------------

	virtual bool Attach()									{ return false; }
	virtual bool Start()									{ return false; }
	virtual void Stop()										{ }

	virtual void StepIn()									{ }
	virtual void StepOver()									{ }
	virtual void StepOut()									{ }

	virtual void Break()									{ }
	virtual void Continue()									{ }

	virtual void AddBreakpoint(const wxString &fileName, unsigned line)		{ }
	virtual void RemoveBreakpoint(const wxString &fileName, unsigned line)	{ }
	virtual void ClearAllBreakpoints()						{ }

	virtual void GetWatchValue(unsigned index, const wxString &variable)	{ }

	// Parse the command sent from the user to the debugger.
	virtual bool OnInterceptInput(const wxString &message)	{ return false; }

	// Parse and possibly respond to the output from the debugger process.
	// Returning true means that the UI won't accept input.
	virtual bool OnOutput(const wxString &message)			{ return false; }
	virtual bool OnError(const wxString &message)			{ return false; }


	// -- User interface ------------------------------------------------------

	const Support &GetSupportedFeatures()					{ return support; }

	// The UI name for this plugin.
	virtual wxString GetName() const						{ return "Custom"; }

	// Build up the full command from the current executable, script and
	// parameters with any extra glue that is necessary.
	virtual wxString GetCommand() const;


	const wxString &GetExecutable() const					{ return executable; }
	const wxString &GetScript() const						{ return script; }
	const wxString &GetArguments() const					{ return arguments; }

	void SetExecutable(const wxString &e)					{ executable = e; }
	void SetScript(const wxString &s)						{ script = s; }
	void SetArguments(const wxString &a)					{ arguments = a; }

	bool UseCustomCommand() const							{ return useCustomCommand; }
	void SetUseCustomCommand(bool use)						{ useCustomCommand = use; }

	const wxString &GetCustomCommand() const				{ return customCommand; }
	void SetCustomCommand(const wxString &c)				{ customCommand = c; }


protected:
	Support		support;

	wxString	executable,
				script,
				arguments,
				customCommand;
	bool		useCustomCommand;
};

#endif // DEBUGGER_H
