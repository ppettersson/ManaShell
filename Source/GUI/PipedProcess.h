#ifndef PIPED_PROCESS_H
#define PIPED_PROCESS_H

#include "wx/process.h"

class MainFrame;

class PipedProcess : public wxProcess
{
public:
	PipedProcess(MainFrame *host);

	virtual void OnTerminate(int pid, int status);
	virtual bool IsActive();

protected:
	MainFrame	*host;
};

#endif // PIPED_PROCESS_H
