#include "MainFrame.h"
#include "PipedProcess.h"

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
	EVT_MENU(kFile_Exit,				MainFrame::OnFileExit)
	EVT_MENU(kView_Breakpoints,			MainFrame::OnViewBreakpoints)
	EVT_MENU(kView_Callstack,			MainFrame::OnViewCallstack)
	EVT_MENU(kView_EditorSource,		MainFrame::OnViewEditorSource)
	EVT_MENU(kView_EditorAssembly,		MainFrame::OnViewEditorAssembly)
	EVT_MENU(kView_EditorMixed,			MainFrame::OnViewEditorMixed)
	EVT_MENU(kView_Registers,			MainFrame::OnViewRegisters)
	EVT_MENU(kView_Watch,				MainFrame::OnViewWatch)
	EVT_MENU(kView_Fullscreen,			MainFrame::OnViewFullscreen)
	EVT_MENU(kDebug_Attach,				MainFrame::OnDebugAttach)
	EVT_MENU(kDebug_Start,				MainFrame::OnDebugStart)
	EVT_MENU(kDebug_Stop,				MainFrame::OnDebugStop)
	EVT_MENU(kDebug_StepIn,				MainFrame::OnDebugStepIn)
	EVT_MENU(kDebug_StepOver,			MainFrame::OnDebugStepOver)
	EVT_MENU(kDebug_StepOut,			MainFrame::OnDebugStepOut)
	EVT_MENU(kDebug_Break,				MainFrame::OnDebugBreak)
	EVT_MENU(kDebug_Continue,			MainFrame::OnDebugContinue)
	EVT_MENU(kDebug_ToggleBreakpoint,	MainFrame::OnDebugToggleBreakpoint)
	EVT_MENU(kTools_Options,			MainFrame::OnToolsOptions)
	EVT_MENU(kHelp_About,				MainFrame::OnHelpAbout)
END_EVENT_TABLE()

MainFrame::MainFrame()
	: wxFrame(NULL, wxID_ANY, "ManaShell - alpha")
	, idleWakeUpTimer(this, kTimer_Idle)
{
	SetupMenu();
	Show();
}

MainFrame::~MainFrame()
{
	// Delete all running processes.
	for (size_t i = 0; i < runningProcesses.size(); ++i)
		delete runningProcesses[i];
}

// Callbacks from running child processes.
void MainFrame::OnProcessTerminated(PipedProcess *process, int pid, int status)
{
	RemovePipedProcess(process);
	delete process;

	//if (pid == converterProcessId)
	//	OnFileImportObjFinished(status);
}

void MainFrame::OnFileExit(wxCommandEvent &event)
{
	Close(true);
}

void MainFrame::OnViewBreakpoints(wxCommandEvent &event)
{
}

void MainFrame::OnViewCallstack(wxCommandEvent &event)
{
}

void MainFrame::OnViewEditorSource(wxCommandEvent &event)
{
}

void MainFrame::OnViewEditorAssembly(wxCommandEvent &event)
{
}

void MainFrame::OnViewEditorMixed(wxCommandEvent &event)
{
}

void MainFrame::OnViewRegisters(wxCommandEvent &event)
{
}

void MainFrame::OnViewWatch(wxCommandEvent &event)
{
}

void MainFrame::OnViewFullscreen(wxCommandEvent &event)
{
	ShowFullScreen(!IsFullScreen(), wxFULLSCREEN_NOTOOLBAR |
				   wxFULLSCREEN_NOSTATUSBAR | wxFULLSCREEN_NOBORDER |
				   wxFULLSCREEN_NOCAPTION);
}

void MainFrame::OnDebugAttach(wxCommandEvent &event)
{
}

void MainFrame::OnDebugStart(wxCommandEvent &event)
{
}

void MainFrame::OnDebugStop(wxCommandEvent &event)
{
}

void MainFrame::OnDebugStepIn(wxCommandEvent &event)
{
}

void MainFrame::OnDebugStepOver(wxCommandEvent &event)
{
}

void MainFrame::OnDebugStepOut(wxCommandEvent &event)
{
}

void MainFrame::OnDebugBreak(wxCommandEvent &event)
{
}

void MainFrame::OnDebugContinue(wxCommandEvent &event)
{
}

void MainFrame::OnDebugToggleBreakpoint(wxCommandEvent &event)
{
}

void MainFrame::OnToolsOptions(wxCommandEvent &event)
{
}

void MainFrame::OnHelpAbout(wxCommandEvent &event)
{
	wxMessageBox("Mana Shell - alpha\n(c)2013 Peter Pettersson. All rights reserved.", "About Mana Shell");
}

void MainFrame::OnIdle(wxCommandEvent &event)
{
	//size_t numProcesses = runningProcesses.GetCount();
	//for (size_t i = 0; i < numProcesses; ++i)
	//	if (runningProcesses[i]->HasInput())
	//		event.RequestMore();
}

void MainFrame::OnTimerIdle(wxTimerEvent &event)
{
	wxWakeUpIdle();
}

void MainFrame::OnClose(wxCloseEvent &event)
{
	// ToDo:
	//if (event.CanVeto() && !fileNotSaved)
	//{
	//	if (wxMessageBox("The file has not been saved... continue closing?", "Please confirm", wxICON_QUESTION | wxYES_NO) != wxYES)
	//	{
	//		event.Veto();
	//		return;
	//	}
	//}

	//wxGetApp().Shutdown();

	event.Skip();
}

void MainFrame::AddPipedProcess(PipedProcess *process)
{
	// Do IO for all child processes in the idle event.
	// This timer is used to kick start that processing.
	if (runningProcesses.IsEmpty())
		idleWakeUpTimer.Start(100);

	runningProcesses.Add(process);
}

void MainFrame::RemovePipedProcess(PipedProcess *process)
{
	runningProcesses.Remove(process);

	// Stop the idle timer if this was the last running process.
	if (runningProcesses.IsEmpty())
		idleWakeUpTimer.Stop();
}

void MainFrame::SetupMenu()
{
	wxMenuBar *menuBar = new wxMenuBar;

	wxMenu *menu = new wxMenu;
	menu->Append(kFile_Exit, "&Exit");
	menuBar->Append(menu, "&File");

	menu = new wxMenu;
	menu->Append(kView_Breakpoints, "&Breakpoints");
	menu->Append(kView_Callstack, "&Callstack");

	wxMenu *subMenu = new wxMenu;
	subMenu->Append(kView_EditorSource, "&Source");
	subMenu->Append(kView_EditorAssembly, "&Assembly");
	subMenu->Append(kView_EditorMixed, "&Mixed");
	menu->AppendSubMenu(subMenu, "&Editor");

	menu->Append(kView_Registers, "&Registers");
	menu->Append(kView_Watch, "&Watch");
	menu->AppendSeparator();
	menu->Append(kView_Fullscreen, "&Fullscreen");
	menuBar->Append(menu, "&View");

	menu = new wxMenu;
	menu->Append(kDebug_Attach, "&Attach");
	menu->Append(kDebug_Start, "&Start");
	menu->Append(kDebug_Stop, "Sto&p");
	menu->AppendSeparator();
	menu->Append(kDebug_StepIn, "Step &In");
	menu->Append(kDebug_StepOver, "Step &Over");
	menu->Append(kDebug_StepOut, "Step Ou&t");
	menu->AppendSeparator();
	menu->Append(kDebug_Break, "&Break");
	menu->Append(kDebug_Continue, "&Continue");
	menu->AppendSeparator();
	menu->Append(kDebug_ToggleBreakpoint, "Toggle &Breakpoint");
	menuBar->Append(menu, "&Debug");

	menu = new wxMenu;
	menu->Append(kTools_Options, "&Options");
	menuBar->Append(menu, "&Tools");

	menu = new wxMenu;
	menu->Append(kHelp_About, "&About");
	menuBar->Append(menu, "&Help");

	SetMenuBar(menuBar);
}
