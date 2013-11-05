#ifndef MAIN_FRAME_H
#define MAIN_FRAME_H

#include "wx/wx.h"
#include "wx/aui/aui.h"
#include "SupportedViews.h"

class Breakpoints;
class Callstack;
class Debugger;
class Input;
class Locals;
class Output;
class PipedProcess;
class Registers;
class SourceEditor;
class Threads;
class Watch;

WX_DEFINE_ARRAY_PTR(PipedProcess *, PipedProcessContainer);

// The main user interface window.
class MainFrame : public wxFrame
{
public:
	MainFrame();
	virtual ~MainFrame();

	// Redo the layout for the UI components.
	void UpdateFrames();

	// Callbacks from running child processes.
	void OnProcessTerminated(PipedProcess *process, int pid, int status);
	void OnOutputFromProcess(const wxString &message);
	void OnErrorFromProcess(const wxString &message);

	void SendCommand(const wxString &command, bool fromUser = false);
	void SendInterrupt();
	void UpdateSource(const wxString &fileName, unsigned line, bool moveDebugMarker = true);
	void DebuggerTermination();

	Callstack *GetCallstack()			{ return callstack; }
	Watch *GetWatch()					{ return watch; }

	void GetWatchValue(unsigned index, const wxString &variable);


private:
	enum Constants
	{
		// Make sure we don't generate any id's that clashes with pre-defined
		// values.
		kUserId						= wxID_HIGHEST + 1000,

		// -- Menu identifiers -----------------------------------------------
		//kFile_Exit,				// Use pre-defined value.

		kView_EditorSource,
		kView_EditorAssembly,
		kView_EditorMixed,
		kView_Breakpoints,
		kView_Callstack,
		kView_Threads,
		kView_Output,
		kView_Input,
		kView_Registers,
		kView_Locals,
		kView_Watch,
		kView_Fullscreen,

		kDebug_Attach,
		kDebug_Start,
		kDebug_Stop,
		kDebug_StepIn,
		kDebug_StepOver,
		kDebug_StepOut,
		kDebug_Break,
		kDebug_Continue,
		kDebug_ToggleBreakpoint,
		kDebug_ClearAllBreakpoints,

		kTools_DryCallstack,
		kTools_Options,

		kHelp_JDB,
		kHelp_GDB,
		kHelp_PDB,
		//kHelp_About,				// Use pre-defined values.

		// -- Special identifiers --------------------------------------------
		kTimer_Idle,

		kFirstMenuId				= kView_EditorSource,
		kLastMenuId					= kTools_Options,


		// -- Pre-defined values ---------------------------------------------
		// These should be defined last to not pollute the enumeration id's.
		kFile_Exit					= wxID_CLOSE,
		kHelp_About					= wxID_ABOUT
	};

	enum SourceEditorMode
	{
		kSource,
		kAssembly,
		kMixed
	};


	// All running processes with redirected Input-Output.
	PipedProcessContainer			runningProcesses;
	long							activeProcessId;

	// One shot timer used to kick start idle processing.
	wxTimer							idleWakeUpTimer;

	// Dockable frame's manager.
	wxAuiManager					dockingManager;

	// Frames.
	Breakpoints						*breakpoints;
	Callstack						*callstack;
	Input							*input;
	Locals							*locals;
	Output							*output;
	Registers						*registers;
	SourceEditor					*sourceEditor;
	Threads							*threads;
	Watch							*watch;

	SourceEditorMode				sourceEditorMode;

	Debugger						*debugger;
	bool							waitingForResponse,
									refocusInput;


	// -- Menu handlers ------------------------------------------------------
	void OnFileExit(wxCommandEvent &event);

	void OnViewEditorSource(wxCommandEvent &event);
	void OnViewEditorAssembly(wxCommandEvent &event);
	void OnViewEditorMixed(wxCommandEvent &event);
	void OnViewBreakpoints(wxCommandEvent &event);
	void OnViewCallstack(wxCommandEvent &event);
	void OnViewThreads(wxCommandEvent &event);
	void OnViewOutput(wxCommandEvent &event);
	void OnViewInput(wxCommandEvent &event);
	void OnViewRegisters(wxCommandEvent &event);
	void OnViewLocals(wxCommandEvent &event);
	void OnViewWatch(wxCommandEvent &event);
	void OnViewFullscreen(wxCommandEvent &event);

	void OnDebugAttach(wxCommandEvent &event);
	void OnDebugStart(wxCommandEvent &event);
	void OnDebugStop(wxCommandEvent &event);
	void OnDebugStepIn(wxCommandEvent &event);
	void OnDebugStepOver(wxCommandEvent &event);
	void OnDebugStepOut(wxCommandEvent &event);
	void OnDebugBreak(wxCommandEvent &event);
	void OnDebugContinue(wxCommandEvent &event);
	void OnDebugToggleBreakpoint(wxCommandEvent &event);
	void OnDebugClearAllBreakpoints(wxCommandEvent &event);

	void OnToolsDryCallstack(wxCommandEvent &event);
	void OnToolsOptions(wxCommandEvent &event);

	void OnHelpJDB(wxCommandEvent &event);
	void OnHelpGDB(wxCommandEvent &event);
	void OnHelpPDB(wxCommandEvent &event);
	void OnHelpAbout(wxCommandEvent &event);


	// -- Event handlers -----------------------------------------------------
	void OnUpdateUI(wxUpdateUIEvent &event);
	void OnIdle(wxIdleEvent &event);
	void OnTimerIdle(wxTimerEvent &event);
	void OnClose(wxCloseEvent &event);


	// -- Process management -------------------------------------------------
	void AddPipedProcess(PipedProcess *process);
	void RemovePipedProcess(PipedProcess *process);


	// -- User interface ------------------------------------------------------
	void SetupMenu();
	void SetupInitialView();
	void UpdateViews(const SupportedViews &views);

	// -- Internal ----------------------------------------------------------


	wxDECLARE_EVENT_TABLE();
};

#endif // MAIN_FRAME_H
