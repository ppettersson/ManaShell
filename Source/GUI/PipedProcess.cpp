#include "MainFrame.h"
#include "PipedProcess.h"
#include "wx/txtstrm.h"


PipedProcess::PipedProcess(MainFrame *h)
	: wxProcess(h)
	, host(h)
{
	Redirect();
}

void PipedProcess::OnTerminate(int pid, int status)
{
	// Capture all the output before we let the process go.
	while (Process())
		;

	host->OnProcessTerminated(this, pid, status);
}

bool PipedProcess::Process()
{
	// Return true if we've processed any input or output.
	// That indicates that we want to be called again soon.
	bool result = false;

	if (!command.IsEmpty())
	{
		wxTextOutputStream os(*GetOutputStream());
		os.WriteString(command);
		command.Empty();

		result = true;
	}

	if (IsErrorAvailable())
	{
		host->OnErrorFromProcess(ReadStream(GetErrorStream()));
		result = true;
	}

	if (IsInputAvailable())
	{
		host->OnOutputFromProcess(ReadStream(GetInputStream()));
		result = true;
	}

	return result;
}

wxString PipedProcess::ReadStream(wxInputStream *stream)
{
	// We can't assume that the output has line breaks and instead have to use
	// a plain buffer and handle line breaks etc on the receiving end.
	const unsigned kBufferSize	= 4096;
	static wxChar buffer[kBufferSize];

	// wxChar may be 2 or 4 bytes and we don't want to read any incomplete
	// characters, that means that we can't use WXSIZEOF.
	const unsigned kBufferBytes	= (kBufferSize - 1) * sizeof(wxChar);

	// Read as much as we can, up to our max buffer size.
	if (stream->CanRead())
	{
		stream->Read(buffer, kBufferBytes);

		// Null terminate the buffer so that we can convert it to a string.
		// ToDo: Not a 100% sure if this works with all encodings?
		size_t lastRead = stream->LastRead();
		buffer[lastRead - 1] = 0;
	}
	else
		buffer[0] = 0;

	// ToDo: wxTextInputStream is using wxConvAuto for encoding conversion.
	// That fails, investigate if this is our bug, a wx bug or a Windows
	// bug.
	return wxString::FromAscii((const char *)buffer);
}

void PipedProcess::SendCommand(const wxString &c)
{
	command = c;
}
