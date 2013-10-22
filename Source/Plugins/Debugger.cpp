#include "../GUI/Dialogs/DebuggerDialog.h"
#include "../GUI/MainFrame.h"
#include "Debugger.h"
#include "PDB/PDB.h"

Debugger *Debugger::Create(MainFrame *host)
{
	std::vector<Debugger *> debuggers;
	//debuggers.push_back(new GDB(host));
	//debuggers.push_back(new JDB(host));
	debuggers.push_back(new PDB(host));
	debuggers.push_back(new Debugger(host));

	DebuggerDialog dialog(host, debuggers);
	if (wxID_OK == dialog.ShowModal())
		return dialog.GetDebugger();

	return NULL;
}

// Build up the full command from the current executable, script and
// parameters with any extra glue that is necessary.
wxString Debugger::GetCommand() const
{
	if (useCustomCommand)
		return customCommand;

	wxString result = executable;
	result += " ";
	result += script;
	result += " ";
	result += arguments;
	return result;
}
