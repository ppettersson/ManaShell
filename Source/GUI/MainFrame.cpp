#include "../Plugins/Debugger.h"
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
#include "wx/utils.h"
#include <algorithm>


BEGIN_EVENT_TABLE(MainFrame, wxFrame)
	EVT_MENU(kFile_Exit,					MainFrame::OnFileExit)
	EVT_MENU(kView_EditorSource,			MainFrame::OnViewEditorSource)
	EVT_MENU(kView_EditorAssembly,			MainFrame::OnViewEditorAssembly)
	EVT_MENU(kView_EditorMixed,				MainFrame::OnViewEditorMixed)
	EVT_MENU(kView_Breakpoints,				MainFrame::OnViewBreakpoints)
	EVT_MENU(kView_Callstack,				MainFrame::OnViewCallstack)
	EVT_MENU(kView_Threads,					MainFrame::OnViewThreads)
	EVT_MENU(kView_Output,					MainFrame::OnViewOutput)
	EVT_MENU(kView_Input,					MainFrame::OnViewInput)
	EVT_MENU(kView_Registers,				MainFrame::OnViewRegisters)
	EVT_MENU(kView_Locals,					MainFrame::OnViewLocals)
	EVT_MENU(kView_Watch,					MainFrame::OnViewWatch)
	EVT_MENU(kView_Fullscreen,				MainFrame::OnViewFullscreen)
	EVT_MENU(kDebug_Attach,					MainFrame::OnDebugAttach)
	EVT_MENU(kDebug_Start,					MainFrame::OnDebugStart)
	EVT_MENU(kDebug_Stop,					MainFrame::OnDebugStop)
	EVT_MENU(kDebug_StepIn,					MainFrame::OnDebugStepIn)
	EVT_MENU(kDebug_StepOver,				MainFrame::OnDebugStepOver)
	EVT_MENU(kDebug_StepOut,				MainFrame::OnDebugStepOut)
	EVT_MENU(kDebug_Break,					MainFrame::OnDebugBreak)
	EVT_MENU(kDebug_Continue,				MainFrame::OnDebugContinue)
	EVT_MENU(kDebug_ToggleBreakpoint,		MainFrame::OnDebugToggleBreakpoint)
	EVT_MENU(kDebug_ClearAllBreakpoints,	MainFrame::OnDebugClearAllBreakpoints)
	EVT_MENU(kTools_DryCallstack,			MainFrame::OnToolsDryCallstack)
	EVT_MENU(kTools_Options,				MainFrame::OnToolsOptions)
	EVT_MENU(kHelp_JDB,						MainFrame::OnHelpJDB)
	EVT_MENU(kHelp_GDB,						MainFrame::OnHelpGDB)
	EVT_MENU(kHelp_PDB,						MainFrame::OnHelpPDB)
	EVT_MENU(kHelp_About,					MainFrame::OnHelpAbout)
	EVT_UPDATE_UI_RANGE(kFirstMenuId,
						kLastMenuId,		MainFrame::OnUpdateUI)
	EVT_IDLE(								MainFrame::OnIdle)
	EVT_TIMER(kTimer_Idle,					MainFrame::OnTimerIdle)
	EVT_CLOSE(								MainFrame::OnClose)
END_EVENT_TABLE()


MainFrame::MainFrame()
	: wxFrame(NULL, wxID_ANY, "ManaShell [alpha]")
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
	, debugger(NULL)
	, waitingForResponse(true)
	, refocusInput(false)
{
	SetClientSize(1600, 900);

	SetupMenu();
	SetupInitialView();

	Show();
	Center();
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
	RemovePipedProcess(process);
	delete process;

	if (pid == activeProcessId)
	{
		activeProcessId = 0;
		output->AppendText("\nProcess was terminated.\n");
		input->Enable(false);

		if (debugger)
		{
			delete debugger;
			debugger = NULL;
		}

		breakpoints->ClearAllBreakpoints();
		callstack->ClearAllFrames();
		watch->ClearAll();
		sourceEditor->StopDebugging();
		sourceEditor->RemoveAllBreakpoints();
	}
}

void MainFrame::OnOutputFromProcess(const wxString &message)
{
	output->AppendText(message);

	bool more = false;
	if (debugger)
		more = debugger->OnOutput(message);

	if (!more)
	{
		waitingForResponse = false;
		input->Enable(true);
		if (refocusInput)
			input->SetFocus();
	}
}

void MainFrame::OnErrorFromProcess(const wxString &message)
{
	// ToDo: Change background color to red instead.
	output->AppendText("\nError: ");
	output->AppendText(message);

	bool more = false;
	if (debugger)
		more = debugger->OnError(message);

	if (!more)
	{
		waitingForResponse = false;
		input->Enable(true);
	}
}

void MainFrame::SendCommand(const wxString &command, bool fromUser)
{
	if (runningProcesses.GetCount())
	{
		input->Enable(false);
		output->AppendText(command);
		if (fromUser && debugger)
			debugger->OnInterceptInput(command);
		runningProcesses[0]->SendCommand(command);
		waitingForResponse = true;
		refocusInput = fromUser;
	}
}

#ifdef __WXMSW__
BOOL WINAPI InterruptHandler(DWORD type)
{
	// Handle this event right here.
	return TRUE;
}
#endif

void MainFrame::SendInterrupt()
{
	if (runningProcesses.GetCount())
	{
		// Process id.
		int pid = runningProcesses[0]->GetPid();
#ifdef __WXMSW__
		if (debugger)
		{
			const DWORD kErrorIcon = wxICON_ERROR | wxOK | wxCENTRE;
			const wxString kErrorCaption = "Send interrupt failed";

			switch (debugger->GetInterruptMethod())
			{
			case Debugger::kDebugBreakProcess:
				{
					HANDLE proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, (DWORD)pid);
					if (!proc)
					{
						wxMessageBox("Error: OpenProcess() failed", kErrorCaption, kErrorIcon, this);
						return;
					}
					if (!DebugBreakProcess(proc))
						wxMessageBox("Error: DebugBreakProcess() failed", kErrorCaption, kErrorIcon, this);
					if (!CloseHandle(proc))
						wxMessageBox("Error: CloseHandle() failed", kErrorCaption, kErrorIcon, this);
				}
				break;

			case Debugger::kGenerateConsoleCtrlEvent:
				// Note: If this code is running inside a debugger then it will
				// break here first. Continue running after the interrupt and
				// it should propagate to the child processes.
				if (!hasConsoleAttached)
				{
					if (!AttachConsole(pid))
					{
						wxMessageBox("Error: AttachConsole() failed", kErrorCaption, kErrorIcon, this);
						return;
					}
					hasConsoleAttached = true;
				}

				if (!hasInterruptHandlerSet)
				{
					if (!SetConsoleCtrlHandler(InterruptHandler, TRUE))
					{
						wxMessageBox("Error: SetConsoleCtrlHandler() failed", kErrorCaption, kErrorIcon, this);
						return;
					}
					hasInterruptHandlerSet = true;
				}

				if (!GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0))
					wxMessageBox("Error: GenerateConsoleCtrlEvent() failed", kErrorCaption, kErrorIcon, this);

				// ToDo? FreeConsole();
				break;
			}
		}
#else
		wxProcess::Kill(pid, wxSIGINT);
#endif
		waitingForResponse = true;
	}
}

void MainFrame::UpdateSource(const wxString &fileName, unsigned line, bool moveDebugMarker)
{
	sourceEditor->Load(fileName, line, moveDebugMarker);
}

void MainFrame::DebuggerTermination()
{
	const SupportedViews views;
	UpdateViews(views);
}

void MainFrame::GetWatchValue(unsigned index, const wxString &variable)
{
	if (debugger)
		debugger->GetWatchValue(index, variable);
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
	//wxString command = ::wxGetTextFromUser("What command should I run?", "Debug start",
	//	//"\"C:/Program Files (x86)/CodeBlocks/MinGW/bin/gdb.exe\" -nw C:/Code/CodeBlocksTest/helloWorld2/bin/Debug/helloWorld2.exe --directory=\"C:/Code/CodeBlocksTest/helloWorld2/\"");
	//	"c:/python33/python.exe -i -m pdb C:/Code/python_test/raytracer.py");
	//if (command.empty())
	//	return;

	// Show a UI to let the user pick one of the debugger plugins
	// and the options to run.
	debugger = Debugger::Create(this);
	if (!debugger)
		return;

	if (!debugger->Start())
		return;

	// Hide frames that aren't supported by this debugger.
	UpdateViews(debugger->GetSupportedFeatures());

	// Show wait cursor to indicate that we're working.
	// Starting a process can take some time.
	wxBusyCursor wait;

	// Remove all output from the last run before we start.
	output->Clear();

	// Run the command and attach to the input and output.
	PipedProcess *process = new PipedProcess(this);
	activeProcessId = wxExecute(debugger->GetCommand(), wxEXEC_ASYNC, process);
	if (!activeProcessId)
	{
		wxMessageBox("Failed to start the debugger", "Error");
		delete process;
		return;
	}

	AddPipedProcess(process);

	input->Enable(true);

#ifdef __WXMSW__
	hasConsoleAttached		= false;
	hasInterruptHandlerSet	= false;
#endif
}

void MainFrame::OnDebugStop(wxCommandEvent &event)
{
	if (debugger)
		debugger->Stop();

	const SupportedViews views;
	UpdateViews(views);
}

void MainFrame::OnDebugStepIn(wxCommandEvent &event)
{
	if (debugger)
		debugger->StepIn();
}

void MainFrame::OnDebugStepOver(wxCommandEvent &event)
{
	if (debugger)
		debugger->StepOver();
}

void MainFrame::OnDebugStepOut(wxCommandEvent &event)
{
	if (debugger)
		debugger->StepOut();
}

void MainFrame::OnDebugBreak(wxCommandEvent &event)
{
	if (debugger)
		debugger->Break();
}

void MainFrame::OnDebugContinue(wxCommandEvent &event)
{
	if (debugger)
		debugger->Continue();
}

void MainFrame::OnDebugToggleBreakpoint(wxCommandEvent &event)
{
	if (debugger)
	{
		wxString fileName = sourceEditor->GetCurrentFile();
		unsigned line = (unsigned)std::max(sourceEditor->GetCurrentLine(), 0);

		// GetCurrentLine is 0-based, while both scintilla and most debuggers are
		// 1-based, so fix up that here.
		++line;

		Breakpoints::ToggleResult result = breakpoints->ToggleBreak(fileName, line);
		switch (result)
		{
		case Breakpoints::kAdded:
			debugger->AddBreakpoint(fileName, line);
			sourceEditor->AddBreakpoint(line);		// Assume it's the current file.
			break;

		case Breakpoints::kRemoved:
			debugger->RemoveBreakpoint(fileName, line);
			sourceEditor->RemoveBreakpoint(line);	// Assume it's the current file.
			break;

		default:
			break;
		}
	}
}

void MainFrame::OnDebugClearAllBreakpoints(wxCommandEvent &event)
{
	if (debugger)
	{
		breakpoints->ClearAllBreakpoints();
		debugger->ClearAllBreakpoints();
		sourceEditor->RemoveAllBreakpoints();		// Assume it's the current file.
	}
}

void MainFrame::OnToolsDryCallstack(wxCommandEvent &event)
{
	// Browse for the file.
	wxString fileName = wxFileSelector("Load a callstack from file", wxEmptyString, wxEmptyString, wxEmptyString,
		"All files (*.*)|*.*",
		wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	// If the dialog was cancelled then this is empty.
	if (fileName.empty())
		return;

	callstack->Load(fileName);
}

void MainFrame::OnToolsOptions(wxCommandEvent &event)
{
}

void MainFrame::OnHelpJDB(wxCommandEvent &event)
{
	wxLaunchDefaultBrowser("http://docs.oracle.com/javase/1.3/docs/tooldocs/solaris/jdb.html");
}

void MainFrame::OnHelpGDB(wxCommandEvent &event)
{
	wxLaunchDefaultBrowser("https://sourceware.org/gdb/current/onlinedocs/gdb/");
}

void MainFrame::OnHelpPDB(wxCommandEvent &event)
{
	wxLaunchDefaultBrowser("http://docs.python.org/3.3/library/pdb.html");
}

void MainFrame::OnHelpAbout(wxCommandEvent &event)
{
	wxMessageBox("Mana Shell 0.3\n\n"
				 "A fast and flexible debugger front-end.\n\n"
				 "(c) 2013 Peter Pettersson.\n\n"
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

	//case kDebug_Attach:
	case kDebug_Start:
		event.Enable(activeProcessId == 0);
		break;

	case kDebug_Stop:
		event.Enable(activeProcessId != 0);
		break;

	case kDebug_Break:
		event.Enable((activeProcessId != 0) && waitingForResponse);
		break;

	case kDebug_StepIn:
	case kDebug_StepOver:
	case kDebug_StepOut:
	case kDebug_Continue:
	case kDebug_ToggleBreakpoint:
	case kDebug_ClearAllBreakpoints:
		event.Enable((activeProcessId != 0) && !waitingForResponse);
		break;
	}
}

void MainFrame::OnIdle(wxIdleEvent &event)
{
	size_t numProcesses = runningProcesses.GetCount();
	for (size_t i = 0; i < numProcesses; ++i)
		if (runningProcesses[i]->Process())
			event.RequestMore();

	event.Skip();
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
	runningProcesses.Add(process);

	// Do IO for all child processes in the idle event.
	// This timer is used to kick start that processing.
	if (!runningProcesses.IsEmpty())
		if (!idleWakeUpTimer.IsRunning())
			idleWakeUpTimer.Start(100);
}

void MainFrame::RemovePipedProcess(PipedProcess *process)
{
	runningProcesses.Remove(process);

	// Stop the idle timer if this was the last running process.
	if (runningProcesses.IsEmpty())
		if (idleWakeUpTimer.IsRunning())
			idleWakeUpTimer.Stop();
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
	menu->Append(kDebug_Attach, "&Attach...")->Enable(false);
	menu->Append(kDebug_Start, "&Start...");
	menu->Append(kDebug_Stop, "Sto&p\tShift+F5");
	menu->AppendSeparator();
	menu->Append(kDebug_StepIn, "Step &In\tF11");
	menu->Append(kDebug_StepOver, "Step &Over\tF10");
	menu->Append(kDebug_StepOut, "Step Ou&t\tShift+F11");
	menu->AppendSeparator();
	menu->Append(kDebug_Break, "&Break\tCtrl+C");
	menu->Append(kDebug_Continue, "&Continue\tF5");
	menu->AppendSeparator();
	menu->Append(kDebug_ToggleBreakpoint, "Toggle &Breakpoint\tF9");
	menu->Append(kDebug_ClearAllBreakpoints, "Clear All Breakpoints\tCtrl+Shift+F9");
	menuBar->Append(menu, "&Debug");

	menu = new wxMenu;
	menu->Append(kTools_DryCallstack, "Dry &Callstack...")->Enable(false);
	menu->AppendSeparator();
	menu->Append(kTools_Options, "&Options...")->Enable(false);
	menuBar->Append(menu, "&Tools");

	menu = new wxMenu;
	menu->Append(kHelp_JDB, "&JDB");
	menu->Append(kHelp_GDB, "&GDB");
	menu->Append(kHelp_PDB, "&PDB");
	menu->AppendSeparator();
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
	outputPane.Left();
	output = new Output(this);
	dockingManager.AddPane(output, outputPane);

	wxAuiPaneInfo inputPane;
	inputPane.Name("input");
	inputPane.Caption("Input Editor");
	inputPane.Left();
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

	// Hide most of the UI by default.
	const SupportedViews support;
	UpdateViews(support);
}

void MainFrame::UpdateViews(const SupportedViews &support)
{
	dockingManager.GetPane(breakpoints).Show(support.breakpoints);
	dockingManager.GetPane(callstack).Show(support.callstack);
	dockingManager.GetPane(registers).Show(support.registers);
	dockingManager.GetPane(threads).Show(support.threads);
	dockingManager.GetPane(watch).Show(support.watch);
	dockingManager.GetPane(locals).Show(support.locals);
	dockingManager.Update();
}
