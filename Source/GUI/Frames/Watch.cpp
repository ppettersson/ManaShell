#include "../MainFrame.h"
#include "Watch.h"

BEGIN_EVENT_TABLE(Watch, wxGrid)
	EVT_GRID_CELL_CHANGED(Watch::OnChanged)
	EVT_SIZE(Watch::OnSize)
	EVT_GRID_COL_SIZE(Watch::OnColSize)
END_EVENT_TABLE()

Watch::Watch(MainFrame *parent)
	: wxGrid(parent, wxID_ANY)
	, host(parent)
	, numVariables(0)
{
	CreateGrid(0, 3);
	HideRowLabels();
	SetColLabelSize(wxGRID_AUTOSIZE);

	// Add the headers.
	SetColLabelValue(0, "Name");
	SetColLabelValue(1, "Value");
	SetColLabelValue(2, "Type");
	SetDefaultColSize(160);
	DisableColResize(2);

	// There will always be one empty cell.
	AddVariable();

}

Watch::~Watch()
{
}

bool Watch::GetNext(unsigned index, wxString &variable)
{
	if (index < variables.size())
	{
		variable = variables[index].name;
		return true;
	}

	return false;
}

void Watch::Update(unsigned index, const wxString &value, const wxString &type)
{
	if (index < variables.size())
	{
		Variable &v = variables[index];
		v.value	= value;
		v.type	= type;

		SetCellValue(index, 1, value);
		SetCellValue(index, 2, type);
	}
}

void Watch::ClearAll()
{
	// Delete all rows except the empty one.
	DeleteRows(0, numVariables - 1);
	numVariables = 1;

	ClearGrid();
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

void Watch::OnChanged(wxGridEvent &event)
{
	int column	= event.GetCol(),
		row		= event.GetRow();
	wxASSERT(column == 0);	// Don't support changing value (yet) or type.

	wxString value = GetCellValue(row, column);
	if (value.IsEmpty())
	{
		// Remove the variable.
		if (row < (int)variables.size())
		{
			variables.erase(variables.begin() + row);

			ClearAll();
			for (size_t i = 0; i < variables.size(); ++i)
			{
				Variable &v = variables[i];
				SetCellValue(i, 0, v.name);
				SetCellValue(i, 1, v.value);
				SetCellValue(i, 2, v.type);
				AddVariable();
			}
		}
	}
	else
	{
		if (row < (int)variables.size())
		{
			Variable &v = variables[row];

			// Change a currently watched variable and reset it.
			v.name 	= value;
			v.value = "";
			v.type  = "";
		}
		else
		{
			// Watch a new variable.
			variables.push_back(Variable(value));

			// Add a new empty row.
			AddVariable();

			wxASSERT((int)variables.size() - 1 == row);
		}

		unsigned index = row;
		host->GetWatchValue(index, value);
	}
}

void Watch::AutoSizeLastCol(int winWidth)
{
	int totalSize = 0;
	for (int i = GetNumberCols() - 1; i >= 0; --i)
		totalSize += GetColSize(i);
	int deltaSize = winWidth - totalSize;
	int lastColSize = (GetColSize(2) + deltaSize > GetColMinimalAcceptableWidth() ? GetColSize(2) + deltaSize : GetColMinimalAcceptableWidth());
	SetColSize(GetNumberCols() - 1, lastColSize);
}

void Watch::OnSize(wxSizeEvent &event)
{
	AutoSizeLastCol(event.GetSize().GetWidth());
	event.Skip();
}

void Watch::OnColSize(wxGridSizeEvent &event)
{
	AutoSizeLastCol(GetSize().GetWidth());
	event.Skip();
}
