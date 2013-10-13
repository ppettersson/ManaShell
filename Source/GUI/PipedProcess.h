#ifndef PIPED_PROCESS_H
#define PIPED_PROCESS_H

#include "wx/process.h"

class MainFrame;

class PipedProcess : public wxProcess
{
public:
	PipedProcess(MainFrame *host);

	virtual void OnTerminate(int pid, int status);

	bool Process();
	void SendCommand(const wxString &c);


protected:
	MainFrame	*host;
	wxString	command;


	wxString ReadStream(wxInputStream *stream);
};

#endif // PIPED_PROCESS_H
