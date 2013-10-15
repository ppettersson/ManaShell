#ifndef PDB_H
#define PDB_H

#include "../Debugger.h"

class PDB : public Debugger
{
public:
	PDB(MainFrame *host);
	virtual ~PDB();

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


private:
	MainFrame	*host;
	bool		quitting;

	void ParseUpdateSource(const wxString &message);
};

#endif // PDB_H
