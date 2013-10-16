#include "../GUI/Dialogs/DebuggerDialog.h"
#include "../GUI/MainFrame.h"
#include "Debugger.h"
#include "PDB/PDB.h"

Debugger *Debugger::Create(MainFrame *host)
{
	wxString command = ::wxGetTextFromUser("What command should I run?", "Debug start",
		//"\"C:/Program Files (x86)/CodeBlocks/MinGW/bin/gdb.exe\" -nw C:/Code/CodeBlocksTest/helloWorld2/bin/Debug/helloWorld2.exe --directory=\"C:/Code/CodeBlocksTest/helloWorld2/\"");
		"c:/python33/python.exe -i -m pdb C:/Code/python_test/raytracer.py");
	if (command.empty())
		return NULL;

	Debugger *debugger = new PDB(host);
	debugger->command = command;
	return debugger;

	//DebuggerDialog dialog(host);
	//int result = dialog.ShowModal();
	//return NULL;
}
