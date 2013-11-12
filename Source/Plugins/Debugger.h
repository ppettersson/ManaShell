#ifndef DEBUGGER_H
#define DEBUGGER_H

#include "wx/wx.h"
#include "../GUI/SupportedViews.h"

class MainFrame;

class Debugger
{
public:
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

	const SupportedViews &GetSupportedFeatures()			{ return support; }

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

	const wxString &GetWorkingDir() const					{ return workingDir; }
	void SetWorkingDir(const wxString &w)					{ workingDir = w; }


#ifdef __WXMSW__
	enum InterruptMethod
	{
		kDebugBreakProcess,
		kGenerateConsoleCtrlEvent
	};

	virtual InterruptMethod GetInterruptMethod() const		{ return kGenerateConsoleCtrlEvent; }
#endif

protected:
	SupportedViews	support;

	wxString		executable,
					script,
					arguments,
					customCommand,
					workingDir;
	bool			useCustomCommand;
};

#endif // DEBUGGER_H
