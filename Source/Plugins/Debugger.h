#ifndef DEBUGGER_H
#define DEBUGGER_H

class MainFrame;
class wxString;

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

	virtual ~Debugger()								{ }

	virtual bool Attach()							{ return false; }
	virtual bool Start()							{ return false; }
	virtual void Stop()								{ }

	virtual void StepIn()							{ }
	virtual void StepOver()							{ }
	virtual void StepOut()							{ }

	virtual void Break()							{ }
	virtual void Continue()							{ }

	virtual void AddBreakpoint(const wxString &fileName, unsigned line)		{ }
	virtual void RemoveBreakpoint(const wxString &fileName, unsigned line)	{ }
	virtual void ClearAllBreakpoints()				{ }

	virtual void OnOutput(const wxString &message)	{ }
	virtual void OnError(const wxString &message)	{ }

	const Support &GetSupportedFeatures()			{ return support; }


protected:
	Support	support;


	Debugger(MainFrame *host)						{ }
};

#endif // DEBUGGER_H
