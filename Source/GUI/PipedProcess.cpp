#include "MainFrame.h"
#include "PipedProcess.h"
#include "wx/txtstrm.h"


PipedProcess::PipedProcess(MainFrame *h)
	: wxProcess(h)
	, host(h)
{
	TRACE_LOG("PipedProcess::PipedProcess()\n");
	Redirect();
}

void PipedProcess::OnTerminate(int pid, int status)
{
	TRACE_LOG("PipedProcess::OnTerminate()\n");

	// Capture all the output before we let the process go.
	while (HasInput())
		;

	host->OnProcessTerminated(this, pid, status);
}

bool PipedProcess::HasInput()
{
	TRACE_LOG("PipedProcess::HasInput()\n");

	bool result = false;
	if (IsInputAvailable())
	{
		wxTextInputStream tis(*GetInputStream());

		// Note: Assumes line buffered output.
		wxString msg;
		msg << tis.ReadLine();

		msg += "\n";
		host->OnOutputFromProcess(msg);
		result = true;
	}
	if (IsErrorAvailable())
	{
		wxTextInputStream tis(*GetErrorStream());

		// Note: Assumes line buffered output.
		wxString msg;
		msg << tis.ReadLine();

		msg += "\n";
		host->OnErrorFromProcess(msg);
		result = true;
	}
	return result;
}
