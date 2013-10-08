#include "SourceEditor.h"

SourceEditor::SourceEditor(wxWindow *parent)
	: wxTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition,
				 wxDefaultSize, wxNO_BORDER | wxTE_MULTILINE |
				 wxTE_READONLY)
{
}

SourceEditor::~SourceEditor()
{
}
