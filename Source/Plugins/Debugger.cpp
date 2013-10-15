#include "Debugger.h"
#include "PDB/PDB.h"

Debugger *Debugger::Create(MainFrame *host)
{
	return new PDB(host);
}
