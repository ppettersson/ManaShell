#include "Threads.h"

Threads::Threads(wxWindow *parent)
	: wxGrid(parent, wxID_ANY)
	, numThreads(0)
	, activeThread(0)
{
	CreateGrid(0, 2);
	HideRowLabels();

	// Add the headers.
	SetColLabelValue(0, "PID");
	SetColLabelValue(1, "Name");
	SetColSize(0, 40);
	SetColSize(1, 160);

	// tmp
	AddThread("123", "main_thread");
	AddThread("6001", "worker_pool_0");
	AddThread("6013", "worker_pool_1");

	UpdateActiveThread();
}

Threads::~Threads()
{
}

void Threads::AddThread(const wxString &id, const wxString &name)
{
	AppendRows();

	SetCellValue(numThreads, 0, id);
	SetCellValue(numThreads, 1, name);

	SetReadOnly(numThreads, 0);
	SetReadOnly(numThreads, 1);

	++numThreads;
}

void Threads::UpdateActiveThread()
{
	for (unsigned i = 0; i < numThreads; ++i)
	{
		wxColour c = (i == activeThread) ? wxColour(255, 255, 0) : GetDefaultCellBackgroundColour();
		for (int x = 0; x < 2; ++x)
			SetCellBackgroundColour(i, x, c);
	}
}
