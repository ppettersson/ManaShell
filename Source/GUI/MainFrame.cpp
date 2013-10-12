#include "Frames/Breakpoints.h"
#include "Frames/Callstack.h"
#include "Frames/Input.h"
#include "Frames/Locals.h"
#include "Frames/Output.h"
#include "Frames/Registers.h"
#include "Frames/SourceEditor.h"
#include "Frames/Threads.h"
#include "Frames/Watch.h"
#include "MainFrame.h"
#include "PipedProcess.h"

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
	EVT_MENU(kFile_Exit,				MainFrame::OnFileExit)
	EVT_MENU(kView_EditorSource,		MainFrame::OnViewEditorSource)
	EVT_MENU(kView_EditorAssembly,		MainFrame::OnViewEditorAssembly)
	EVT_MENU(kView_EditorMixed,			MainFrame::OnViewEditorMixed)
	EVT_MENU(kView_Breakpoints,			MainFrame::OnViewBreakpoints)
	EVT_MENU(kView_Callstack,			MainFrame::OnViewCallstack)
	EVT_MENU(kView_Threads,				MainFrame::OnViewThreads)
	EVT_MENU(kView_Output,				MainFrame::OnViewOutput)
	EVT_MENU(kView_Input,				MainFrame::OnViewInput)
	EVT_MENU(kView_Registers,			MainFrame::OnViewRegisters)
	EVT_MENU(kView_Locals,				MainFrame::OnViewLocals)
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
	EVT_UPDATE_UI_RANGE(kFirstMenuId,
						kLastMenuId,	MainFrame::OnUpdateUI)
	EVT_IDLE(							MainFrame::OnIdle)
	EVT_TIMER(kTimer_Idle,				MainFrame::OnTimerIdle)
	EVT_CLOSE(							MainFrame::OnClose)
END_EVENT_TABLE()


MainFrame::MainFrame()
	: wxFrame(NULL, wxID_ANY, "ManaShell - alpha")
	, activeProcessId(0)
	, idleWakeUpTimer(this, kTimer_Idle)
	, breakpoints(NULL)
	, callstack(NULL)
	, input(NULL)
	, output(NULL)
	, registers(NULL)
	, sourceEditor(NULL)
	, threads(NULL)
	, watch(NULL)
	, sourceEditorMode(kSource)
{
	SetClientSize(1280, 720);

	SetupMenu();
	SetupInitialView();

	Show();
}

MainFrame::~MainFrame()
{
	dockingManager.UnInit();

	// Delete all running processes.
	for (size_t i = 0; i < runningProcesses.size(); ++i)
		delete runningProcesses[i];
}

// Redo the layout for the UI components.
void MainFrame::UpdateFrames()
{
	dockingManager.Update();
}

// Callbacks from running child processes.
void MainFrame::OnProcessTerminated(PipedProcess *process, int pid, int status)
{
	TRACE_LOG("MainFrame::OnProcessTerminated()\n");
	RemovePipedProcess(process);
	delete process;

	if (pid == activeProcessId)
	{
		TRACE_LOG(" - it was the active process\n");
		activeProcessId = 0;
	}
}

void MainFrame::OnOutputFromProcess(const wxString &message)
{
	TRACE_LOG("MainFrame::OnOutputFromProcess()\n");
	output->AppendText(message);
}

void MainFrame::OnErrorFromProcess(const wxString &message)
{
	TRACE_LOG("MainFrame::OnErrorFromProcess()\n");
	output->AppendText("Error: ");
	output->AppendText(message);
}

void MainFrame::OnFileExit(wxCommandEvent &event)
{
	Close(true);
}

void MainFrame::OnViewEditorSource(wxCommandEvent &event)
{
	sourceEditorMode = kSource;
}

void MainFrame::OnViewEditorAssembly(wxCommandEvent &event)
{
	sourceEditorMode = kAssembly;
}

void MainFrame::OnViewEditorMixed(wxCommandEvent &event)
{
	sourceEditorMode = kMixed;
}

void MainFrame::OnViewBreakpoints(wxCommandEvent &event)
{
	wxAuiPaneInfo &pane = dockingManager.GetPane(breakpoints);
	pane.Show(!pane.IsShown());
	dockingManager.Update();
}

void MainFrame::OnViewCallstack(wxCommandEvent &event)
{
	wxAuiPaneInfo &pane = dockingManager.GetPane(callstack);
	pane.Show(!pane.IsShown());
	dockingManager.Update();
}

void MainFrame::OnViewThreads(wxCommandEvent &event)
{
	wxAuiPaneInfo &pane = dockingManager.GetPane(threads);
	pane.Show(!pane.IsShown());
	dockingManager.Update();
}

void MainFrame::OnViewOutput(wxCommandEvent &event)
{
	wxAuiPaneInfo &pane = dockingManager.GetPane(output);
	pane.Show(!pane.IsShown());
	dockingManager.Update();
}

void MainFrame::OnViewInput(wxCommandEvent &event)
{
	wxAuiPaneInfo &pane = dockingManager.GetPane(input);
	pane.Show(!pane.IsShown());
	dockingManager.Update();
}

void MainFrame::OnViewRegisters(wxCommandEvent &event)
{
	wxAuiPaneInfo &pane = dockingManager.GetPane(registers);
	pane.Show(!pane.IsShown());
	dockingManager.Update();
}

void MainFrame::OnViewLocals(wxCommandEvent &event)
{
	wxAuiPaneInfo &pane = dockingManager.GetPane(locals);
	pane.Show(!pane.IsShown());
	dockingManager.Update();
}

void MainFrame::OnViewWatch(wxCommandEvent &event)
{
	wxAuiPaneInfo &pane = dockingManager.GetPane(watch);
	pane.Show(!pane.IsShown());
	dockingManager.Update();
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
	// The UI shouldn't allow more than one process to be active but guard
	// against accidents.
	if (activeProcessId)
		return;

	// Get the command to run.
	// ToDo...
	wxString command = ::wxGetTextFromUser("What command should I run?", "Debug start",
		"c:/code/orc4/engine/platform/windows/vs2012/output/objconverter.exe");
	if (command.empty())
		return;

	// Show wait cursor to indicate that we're working.
	// Starting a process can take some time.
	wxBusyCursor wait;

	// Remove all output from the last run before we start.
	output->Clear();

	// Run the command and attach to the input and output.
	PipedProcess *process = new PipedProcess(this);
	activeProcessId = wxExecute(command, wxEXEC_ASYNC, process);
	if (!activeProcessId)
	{
		wxMessageBox("Failed to start the debugger", "Error");
		delete process;
		return;
	}

	AddPipedProcess(process);
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
	wxMessageBox("Mana Shell 0.1\n\n"
				 "A fast and flexible debugger front-end.\n\n"
				 "(c) 2013 Peter Pettersson. All rights reserved.\n\n"
				 "https://github.com/ppettersson/ManaShell",
				 "About Mana Shell");
}

void MainFrame::OnUpdateUI(wxUpdateUIEvent &event)
{
	switch (event.GetId())
	{
	case kView_EditorSource:	event.Check(sourceEditorMode == kSource);					break;
	case kView_EditorAssembly:	event.Check(sourceEditorMode == kAssembly); event.Enable(false); break;
	case kView_EditorMixed:		event.Check(sourceEditorMode == kMixed); event.Enable(false); break;
	case kView_Breakpoints:		event.Check(dockingManager.GetPane(breakpoints).IsShown());	break;
	case kView_Callstack:		event.Check(dockingManager.GetPane(callstack).IsShown());	break;
	case kView_Threads:			event.Check(dockingManager.GetPane(threads).IsShown());		break;
	case kView_Output:			event.Check(dockingManager.GetPane(output).IsShown());		break;
	case kView_Input:			event.Check(dockingManager.GetPane(input).IsShown());		break;
	case kView_Registers:		event.Check(dockingManager.GetPane(registers).IsShown());	break;
	case kView_Locals:			event.Check(dockingManager.GetPane(locals).IsShown());		break;
	case kView_Watch:			event.Check(dockingManager.GetPane(watch).IsShown());		break;
	case kView_Fullscreen:		event.Check(IsFullScreen());								break;

	case kDebug_Attach:
	case kDebug_Start:
		event.Enable(activeProcessId == 0);
		break;

	case kDebug_Stop:
	case kDebug_StepIn:
	case kDebug_StepOver:
	case kDebug_StepOut:
	case kDebug_Break:
	case kDebug_Continue:
	case kDebug_ToggleBreakpoint:
		event.Enable(activeProcessId != 0);
		break;
	}
}

void MainFrame::OnIdle(wxIdleEvent &event)
{
	size_t numProcesses = runningProcesses.GetCount();
	for (size_t i = 0; i < numProcesses; ++i)
		if (runningProcesses[i]->IsActive())
		{
			TRACE_LOG("MainFrame::OnIdle() - requesting more\n");
			event.RequestMore();
		}

	event.Skip();
}

void MainFrame::OnTimerIdle(wxTimerEvent &event)
{
	TRACE_LOG("MainFrame::OnTimerIdle()\n");
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
	TRACE_LOG("MainFrame::AddPipedProcess()\n");

	runningProcesses.Add(process);

	// Do IO for all child processes in the idle event.
	// This timer is used to kick start that processing.
	if (runningProcesses.IsEmpty())
	{
		TRACE_LOG(" - starting the idle wake up timer\n");
		if (!idleWakeUpTimer.IsRunning())
			idleWakeUpTimer.Start(100);
	}
}

void MainFrame::RemovePipedProcess(PipedProcess *process)
{
	TRACE_LOG("MainFrame::RemovePipedProcess()\n");

	runningProcesses.Remove(process);

	// Stop the idle timer if this was the last running process.
	if (runningProcesses.IsEmpty())
	{
		TRACE_LOG(" - stopping the idle wake up timer\n");
		if (idleWakeUpTimer.IsRunning())
			idleWakeUpTimer.Stop();
	}
}

void MainFrame::SetupMenu()
{
	wxMenuBar *menuBar = new wxMenuBar;

	// File.
	wxMenu *menu = new wxMenu;
	menu->Append(kFile_Exit, "&Exit");
	menuBar->Append(menu, "&File");

	// View.
	menu = new wxMenu;

	wxMenu *subMenu = new wxMenu;
	subMenu->AppendCheckItem(kView_EditorSource, "&Source");
	subMenu->AppendCheckItem(kView_EditorAssembly, "&Assembly");
	subMenu->AppendCheckItem(kView_EditorMixed, "&Mixed");
	menu->AppendSubMenu(subMenu, "&Editor");

	subMenu = new wxMenu;
	subMenu->AppendCheckItem(kView_Breakpoints, "&Breakpoints");
	subMenu->AppendCheckItem(kView_Callstack, "&Callstack");
	subMenu->AppendCheckItem(kView_Threads, "&Threads");
	subMenu->AppendCheckItem(kView_Output, "&Output");
	subMenu->AppendCheckItem(kView_Input, "&Input");
	subMenu->AppendCheckItem(kView_Registers, "&Registers");
	subMenu->AppendCheckItem(kView_Locals, "&Locals");
	subMenu->AppendCheckItem(kView_Watch, "&Watch");
	menu->AppendSubMenu(subMenu, "&Frames");

	menu->AppendSeparator();
	menu->AppendCheckItem(kView_Fullscreen, "&Fullscreen");
	menuBar->Append(menu, "&View");

	menu = new wxMenu;
	menu->Append(kDebug_Attach, "&Attach...");
	menu->Append(kDebug_Start, "&Start...");
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
	menu->Append(kTools_Options, "&Options...")->Enable(false);
	menuBar->Append(menu, "&Tools");

	menu = new wxMenu;
	menu->Append(kHelp_About, "&About...");
	menuBar->Append(menu, "&Help");

	SetMenuBar(menuBar);
}

void MainFrame::SetupInitialView()
{
	wxSize clientSize = GetClientSize();

	dockingManager.SetManagedWindow(this);

	wxAuiPaneInfo sourceEditorPane;
	sourceEditorPane.Name("sourceEditor");
	sourceEditorPane.Caption("Source Code Viewer");
	sourceEditorPane.Center();
	sourceEditor = new SourceEditor(this);
	dockingManager.AddPane(sourceEditor, sourceEditorPane);

	wxAuiPaneInfo localsPane;
	localsPane.Name("locals");
	localsPane.Caption("Local Variables");
	localsPane.Bottom();
	locals = new Locals(this);
	dockingManager.AddPane(locals, localsPane);

	wxAuiPaneInfo watchPane;
	watchPane.Name("watch");
	watchPane.Caption("Watch");
	watchPane.Bottom();
	watch = new Watch(this);
	dockingManager.AddPane(watch, watchPane);

	wxAuiPaneInfo outputPane;
	outputPane.Name("output");
	outputPane.Caption("Output Log");
	outputPane.Bottom();
	output = new Output(this);
	dockingManager.AddPane(output, outputPane);

	wxAuiPaneInfo inputPane;
	inputPane.Name("input");
	inputPane.Caption("Input Editor");
	inputPane.Bottom();
	input = new Input(this);
	dockingManager.AddPane(input, inputPane);

	wxAuiPaneInfo callstackPane;
	callstackPane.Name("callstack");
	callstackPane.Caption("Callstack");
	callstackPane.Right();
	callstack = new Callstack(this);
	dockingManager.AddPane(callstack, callstackPane);

	wxAuiPaneInfo threadsPane;
	threadsPane.Name("threads");
	threadsPane.Caption("Threads");
	threadsPane.Right();
	threads = new Threads(this);
	dockingManager.AddPane(threads, threadsPane);

	wxAuiPaneInfo registersPane;
	registersPane.Name("registers");
	registersPane.Caption("Registers");
	registersPane.Right();
	registers = new Registers(this);
	dockingManager.AddPane(registers, registersPane);

	wxAuiPaneInfo breakpointsPane;
	breakpointsPane.Name("breakpoints");
	breakpointsPane.Caption("Breakpoints");
	breakpointsPane.Right();
	breakpoints = new Breakpoints(this);
	dockingManager.AddPane(breakpoints, breakpointsPane);

	// dbg
	sourceEditor->LoadFile("C:/Code/ManaShell/Source/GUI/MainFrame.cpp");
	output->LoadFile("C:/Code/ManaShell/Build/VS2012/x64/Debug/cl.command.1.tlog");
	// end

	dockingManager.Update();
}
