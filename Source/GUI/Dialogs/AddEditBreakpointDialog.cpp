#include "AddEditBreakpointDialog.h"
#include "wx/valgen.h"

BEGIN_EVENT_TABLE(AddEditBreakpointDialog, wxDialog)
	EVT_UPDATE_UI(wxID_OK, AddEditBreakpointDialog::OnUpdateUI)
END_EVENT_TABLE()

AddEditBreakpointDialog::AddEditBreakpointDialog(wxWindow *parent, const wxString &f, int l)
	: wxDialog(parent, wxID_ANY, "", wxDefaultPosition,
			   wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
	, fileName(f)
	, lineNumber(l)
{
	// Determine the dialog caption from the arguments.
	if (fileName.IsEmpty() && lineNumber == 0)
		SetTitle("Add breakpoint");
	else
		SetTitle("Edit breakpoint");

	wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
	this->SetSizer(topSizer);

	// The filename.
	wxBoxSizer *fileNameSizer = new wxBoxSizer(wxHORIZONTAL);
	topSizer->Add(fileNameSizer, 0, wxGROW | wxALL, 5);
	{
		wxStaticText *staticText = new wxStaticText(this, wxID_STATIC, "Filename:");
		fileNameSizer->Add(staticText, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

		fileNameSizer->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

		fileNameControl = new wxTextCtrl(this, kFileNameId);
		fileNameControl->SetValidator(wxTextValidator(wxFILTER_NONE, &fileName));
		fileNameSizer->Add(fileNameControl, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);

		wxButton *fileNameBrowse = new wxButton(this, kFileNameBrowseId, "...");
		fileNameBrowse->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AddEditBreakpointDialog::OnBrowse, this);
		fileNameSizer->Add(fileNameBrowse, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
	}

	// The line number.
	wxBoxSizer *lineNumberSizer = new wxBoxSizer(wxHORIZONTAL);
	topSizer->Add(lineNumberSizer, 0, wxGROW | wxALL, 5);
	{
		wxStaticText *staticText = new wxStaticText(this, wxID_STATIC, "Line number:");
		lineNumberSizer->Add(staticText, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

		lineNumberSizer->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

		wxSpinCtrl *lineNumberControl = new wxSpinCtrl(this, wxID_ANY, "0");
		lineNumberControl->SetRange(0, INT_MAX);	// Unlimited max value.
		lineNumberControl->SetValidator(wxGenericValidator(&lineNumber));
		lineNumberSizer->Add(lineNumberControl, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);
	}

	// Spacer.
	topSizer->Add(5, 5, 1, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

	// Regular dialog dismissal.
	wxStdDialogButtonSizer *dialogButtons = new wxStdDialogButtonSizer;
	dialogButtons->AddButton(new wxButton(this, wxID_OK, "&OK"));
	dialogButtons->AddButton(new wxButton(this, wxID_CANCEL, "&Cancel"));
	dialogButtons->Realize();
	topSizer->Add(dialogButtons, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

	topSizer->SetMinSize(wxSize(600, -1));
	SetSizerAndFit(topSizer);
	Center();
}

AddEditBreakpointDialog::~AddEditBreakpointDialog()
{
}

void AddEditBreakpointDialog::OnBrowse(wxCommandEvent &event)
{
	// Browse for the specific file.
	wxString result = wxFileSelector("Choose a file", wxEmptyString, wxEmptyString, wxEmptyString,
		"All files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	// If the dialog was cancelled then this is empty.
	if (!result.IsEmpty())
	{
		fileName = result;
		TransferDataToWindow();
	}
}

void AddEditBreakpointDialog::OnUpdateUI(wxUpdateUIEvent &event)
{
	if (event.GetId() == wxID_OK)
	{
		fileName = fileNameControl->GetValue();
		if (!fileName.IsEmpty())
		{
			bool result = wxFileExists(fileName);
			if (result)
			{
				event.Enable(true);
				return;
			}
		}
	}

	event.Enable(false);
}
