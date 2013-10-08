#include "Watch.h"
#include "wx/grid.h"

Watch::Watch(wxWindow *parent)
	: wxPanel(parent)
{
	wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

	grid = new wxGrid(this, wxID_ANY);
	grid->CreateGrid(4, 3);
	grid->HideRowLabels();
	grid->SetColLabelValue(0, "Variable");
	grid->SetColLabelValue(1, "Value");
	grid->SetColLabelValue(2, "Type");
	topSizer->Add(grid, 0, wxGROW | wxALL, 0);

	SetSizerAndFit(topSizer);
}

Watch::~Watch()
{
}
