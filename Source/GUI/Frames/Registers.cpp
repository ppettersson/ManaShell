#include "Registers.h"

Registers::Registers(wxWindow *parent)
	: wxGrid(parent, wxID_ANY)
	, numRegisters(0)
{
	CreateGrid(0, 2);
	HideRowLabels();

	// Add the headers.
	SetColLabelValue(0, "Register");
	SetColLabelValue(1, "Value");

	AddRegister("EAX", "0x00000000");
	AddRegister("EBX", "0x11111111");
	AddRegister("ECX", "0x22222222");
	AddRegister("EDX", "0x44444444");
}

Registers::~Registers()
{
}

void Registers::AddRegister(const wxString &name, const wxString &value)
{
	AppendRows();

	SetCellValue(numRegisters, 0, name);
	SetCellValue(numRegisters, 1, value);

	SetReadOnly(numRegisters, 0);
	SetReadOnly(numRegisters, 1);

	++numRegisters;
}
