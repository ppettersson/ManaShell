#ifndef PDB_H
#define PDB_H

#include "../Debugger.h"

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
	MainFrame	*host;
	bool		quitting;


	void ParseUpdateSource(const wxString &message);
};

#endif // PDB_H
