#ifndef MAIN_FRAME_H
#define MAIN_FRAME_H

#include "wx/wx.h"

class PipedProcess;

WX_DEFINE_ARRAY_PTR(PipedProcess *, PipedProcessContainer);

// The main user interface window.
class MainFrame : public wxFrame
{
public:
	MainFrame();
	virtual ~MainFrame();

	// Callbacks from running child processes.
	void OnProcessTerminated(PipedProcess *process, int pid, int status);


private:
	enum Constants
	{
		// Make sure we don't generate any id's that clashes with pre-defined
		// values.
		kUserId						= wxID_HIGHEST + 1000,

		// -- Menu identifiers -----------------------------------------------
		//kFile_Exit,				// Use pre-defined value.

		kView_Breakpoints,
		kView_Callstack,
		kView_EditorSource,
		kView_EditorAssembly,
		kView_EditorMixed,
		kView_Registers,
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

		kTools_Options,

		//kHelp_About,				// Use pre-defined values.

		// -- Special identifiers --------------------------------------------
		kTimer_Idle,

		// -- Pre-defined values ---------------------------------------------
		// These should be defined last to not pollute the enumeration id's.
		kFile_Exit					= wxID_CLOSE,
		kHelp_About					= wxID_ABOUT
	};


	// All running processes with redirected Input-Output.
	PipedProcessContainer			runningProcesses;

	// One shot timer used to kick start idle processing.
	wxTimer							idleWakeUpTimer;


	// -- Menu handlers ------------------------------------------------------
	void OnFileExit(wxCommandEvent &event);

	void OnViewBreakpoints(wxCommandEvent &event);
	void OnViewCallstack(wxCommandEvent &event);
	void OnViewEditorSource(wxCommandEvent &event);
	void OnViewEditorAssembly(wxCommandEvent &event);
	void OnViewEditorMixed(wxCommandEvent &event);
	void OnViewRegisters(wxCommandEvent &event);
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

	void OnToolsOptions(wxCommandEvent &event);

	void OnHelpAbout(wxCommandEvent &event);


	// -- Event handlers -----------------------------------------------------
	void OnIdle(wxCommandEvent &event);
	void OnTimerIdle(wxTimerEvent &event);
	void OnClose(wxCloseEvent &event);


	// -- Process management -------------------------------------------------
	void AddPipedProcess(PipedProcess *process);
	void RemovePipedProcess(PipedProcess *process);


	// -- User interface ------------------------------------------------------
	void SetupMenu();


	// -- Internal ----------------------------------------------------------


	wxDECLARE_EVENT_TABLE();
};

#endif // MAIN_FRAME_H
