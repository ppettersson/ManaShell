#include "Watch.h"

Watch::Watch(wxWindow *parent)
	: wxGrid(parent, wxID_ANY, wxDefaultPosition, wxSize(-1, 200))
	, numVariables(0)
{
	CreateGrid(0, 3);
	HideRowLabels();

	// Add the headers.
	SetColLabelValue(0, "Name");
	SetColLabelValue(1, "Value");
	SetColLabelValue(2, "Type");
	SetDefaultColSize(160);

	// tmp
	AddVariable("my_variable", "10", "int");
	AddVariable("foo", "0xdeadbeef", "void *");
	AddVariable("bar", "\"this is a string\"", "char *");
}

Watch::~Watch()
{
}

void Watch::AddVariable(const wxString &name, const wxString &value, const wxString &type)
{
	AppendRows();

	SetCellValue(numVariables, 0, name);
	SetCellValue(numVariables, 1, value);
	SetCellValue(numVariables, 2, type);

	// Value and type is always read-only (for now).
	SetReadOnly(numVariables, 1);
	SetReadOnly(numVariables, 2);

	++numVariables;
}