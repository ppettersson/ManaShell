#include "../GUI/Dialogs/DebuggerDialog.h"
#include "../GUI/MainFrame.h"
#include "Debugger.h"
#include "GDB/GDB.h"
#include "PDB/PDB.h"
#include "wx/cmdline.h"

static void InitAllPlugins(std::vector<Debugger *> &debuggers, MainFrame *host)
{
	debuggers.push_back(new GDB(host));
	//debuggers.push_back(new JDB(host));
	debuggers.push_back(new PDB(host));
	debuggers.push_back(new Debugger(host));
}

Debugger *Debugger::Create(MainFrame *host)
{
	std::vector<Debugger *> debuggers;
	InitAllPlugins(debuggers, host);

	DebuggerDialog dialog(host, debuggers);
	if (wxID_OK == dialog.ShowModal())
		return dialog.GetDebugger();

	return NULL;
}

Debugger *Debugger::Create(wxCmdLineParser &parser)
{
	std::vector<Debugger *> debuggers;
	InitAllPlugins(debuggers, NULL);

	do
	{
		wxString plugin;
		if (!parser.Found("p", &plugin))
			break;

		Debugger *debugger = NULL;
		for (std::vector<Debugger *>::iterator i = debuggers.begin(); i != debuggers.end(); ++i)
		{
			if ((*i)->Select(plugin))
			{
				debugger = *i;
				break;
			}
		}

		if (!debugger)
			break;

		wxString executable;
		if (parser.Found("e", &executable))
			debugger->SetExecutable(executable);

		wxString script;
		if (parser.Found("s", &script))
			debugger->SetScript(script);

		// ToDo: script arguments.

		wxString custom;
		if (parser.Found("c", &custom))
		{
			debugger->SetUseCustomCommand(true);
			debugger->SetCustomCommand(custom);
		}

		wxString workingDir;
		if (parser.Found("w", &workingDir))
			debugger->SetWorkingDir(workingDir);

		return debugger;
	} while (0);

	// Failed to initalize, clean up.
	for (std::vector<Debugger *>::iterator i = debuggers.begin(); i != debuggers.end(); ++i)
		delete *i;
	return NULL;
}

// Build up the full command from the current executable, script and
// parameters with any extra glue that is necessary.
wxString Debugger::GetCommand() const
{
	if (useCustomCommand)
		return customCommand;

	wxString result = executable;
	if (!script.IsEmpty())
	{
		result += " ";
		result += script;
	}
	if (!arguments.IsEmpty())
	{
		result += " ";
		result += arguments;
	}
	return result;
}
