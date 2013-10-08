#include "MainFrame.h"
#include "ManaShell.h"

wxIMPLEMENT_APP(ManaShellApp);

ManaShellApp::ManaShellApp()
	: mainFrame(NULL)
{
}

ManaShellApp::~ManaShellApp()
{
}

bool ManaShellApp::OnInit()
{
#if defined(_DEBUG) && defined(_WIN32)
	// Use this to manually track down (some) memory leaks reported by the run-time library.
	// Unfortunately there's a lot of non-deterministic stuff that makes this hard.
	//_CrtSetBreakAlloc(123);
#endif

	if (!wxApp::OnInit())
		return false;

	mainFrame = new MainFrame();
	return true;
}

int ManaShellApp::OnExit()
{
	return wxApp::OnExit();
}

void ManaShellApp::OnInitCmdLine(wxCmdLineParser &parser)
{
}

bool ManaShellApp::OnCmdLineParsed(wxCmdLineParser &parser)
{
	return true;
}
