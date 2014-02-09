#include "../Plugins/Debugger.h"
#include "MainFrame.h"
#include "ManaShell.h"
#include <wx/cmdline.h>

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

	if (debugger)
	{
		mainFrame->StartDebugSession(debugger);
		debugger = NULL;
	}

	return true;
}

void ManaShellApp::OnInitCmdLine(wxCmdLineParser &parser)
{
	wxAppConsole::OnInitCmdLine(parser);

	parser.SetLogo("ManaShell is a GUI front end for various command line interface debuggers.\n");
	parser.SetSwitchChars("-");

	parser.AddOption("p", "plugin", "start a debug session");
	parser.AddOption("e", "executable", "debugger executable to use");
	parser.AddOption("s", "script", "script to debug");
	parser.AddOption("c", "custom", "use custom command");
	parser.AddOption("w", "workingdir", "set working directory for the debugger");
}

bool ManaShellApp::OnCmdLineParsed(wxCmdLineParser &parser)
{
	// Check if we should start a debug session immediately.
	if (parser.Found("p"))
	{
		debugger = Debugger::Create(parser);
		if (!debugger)
			return false;
	}

	return wxAppConsole::OnCmdLineParsed(parser);
}
